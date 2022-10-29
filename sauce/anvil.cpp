#define SOKOL_LOG(msg) OutputDebugString(msg)
#define SOKOL_D3D11
#define SOKOL_IMPL
#include "third_party/sokol_gfx.h"
#include "third_party/sokol_gp.h"
#include "third_party/sokol_app.h"
#include "third_party/sokol_glue.h"
#include "third_party/sokol_time.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include "third_party/HandmadeMath.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#include "thomas.h"
#include "anvil.h"

#include "cgame.c"

/*

- [x] write a mini portable memory arena using malloc
- [ ] get WASM build and figure out if I can use 64-bit shit?

### On entities
Pulling them out into memory arenas and giving them defined lifetimes was a mistake. Keep it simple, what was the feature I needed - I needed to have a timer on an entity for destruction, and for it to last one frame. Just make that an entity feature!!

As for entities that exist on the main menu, just have a seperate batch ya silly fukin goose.

KEEP IT SIMPLE.

*/

static void frame(void)
{
	Swap(M_Arena, game_memory.frame_arena, game_memory.last_frame_arena);
	M_ArenaClear(&game_memory.frame_arena);
	
	GameState* gs = game_state();
	WorldState* world = world_state();
	if (gs->key_pressed[SAPP_KEYCODE_B])
	{
		MemoryZeroStruct(world);
		WorldInit(world);
	}

	FrameState frame = { 0 };
	frame.player = world->player_temp;

	// zero frame data
	ForEachFlat(entity, world->entities, Entity*)
	{
		MemoryZeroStruct(&entity->frame);
	}

	const Vec2 window_size = { (F32)sapp_width(), (F32)sapp_height() };
	gs->window_size = window_size;
	F32 ratio = window_size.x / (F32)window_size.y;
	F32 delta_t = sapp_frame_duration();
	gs->delta_t = delta_t;
	const Vec2 world_mouse = mouse_pos_in_worldspace();

	// PLAYER INPUT
	if (frame.player) {
		if (gs->key_pressed[SAPP_KEYCODE_SPACE]) {
			frame.player->vel.y = 300.0f;
		}
		Vec2 axis_input = { 0 };
		if (gs->key_down[SAPP_KEYCODE_A]) {
			axis_input.x -= 1.0f;
			frame.player->x_dir = -1;
		}
		if (gs->key_down[SAPP_KEYCODE_D]) {
			axis_input.x += 1.0f;
			frame.player->x_dir = 1;
		}
		frame.player->acc = axis_input * MOVE_SPEED;
	}

	// Entity Physics
	ForEachFlat(entity, world->entities, Entity*) {
		if (!entity->rigid_body)
			continue;

		// acc counter force with existing velocity
		entity->acc.x += -entity->x_friction_mult * entity->vel.x;

		// gravity
		B8 falling = entity->vel.y < 0.f;
		entity->acc.y -= (falling ? 2.f : 1.f) * GRAVITY;

		// integrate acceleration and velocity into position
		Vec2 next_pos = 0.5f * entity->acc * SQUARE(delta_t) + entity->vel * delta_t + entity->pos;

		// integrate acceleration into velocity
		entity->vel += entity->acc * delta_t;
		entity->acc.x = 0;
		entity->acc.y = 0;

		if (next_pos.y < 0.0f) {
			next_pos.y = 0.0f;
			entity->vel.y = 0.0f;
		}

		entity->pos = next_pos;
	}

	// BEGIN RENDER
	sgp_begin(window_size.x, window_size.y);
	sgp_viewport(0, 0, window_size.x, window_size.y);

	#if 1
	sgp_project(window_size.x * -0.5f, window_size.x * 0.5f, window_size.y * 0.5f, window_size.y * -0.5f);
	sgp_scale(gs->cam.scale, gs->cam.scale);
	sgp_translate(-gs->cam.pos.x, -gs->cam.pos.y);
	#else
	Rng2F32 view_rect = { 0 };
	view_rect.max = Vec2(window_size.x, window_size.y);
	view_rect = range2_center_middle(view_rect);
	view_rect = Shift2F32(view_rect, Vec2(0, 100.f));
	view_rect = range2_scale(view_rect, gs->cam.scale);
	view_rect = Shift2F32(view_rect, gs->cam.pos);
	sgp_project(view_rect.min.x, view_rect.max.x, view_rect.max.y, view_rect.min.y);
	#endif

	// @sgp_helpers - vector expander, so I can use my types
	sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
	sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
	sgp_clear();

	// PLAYER POST-PHYSICS UPDATE
	if (frame.player) {
		// camera update
		gs->cam.pos.x = frame.player->pos.x;
		gs->cam.pos.y = 20.0f;

		// INTERACTION RECT
		Rng2F32 interact_rect = { 0 };
		interact_rect.max = Vec2(20, 20);
		if (frame.player->x_dir == -1)
			interact_rect = Flip2F32(interact_rect);
		interact_rect = Shift2F32(interact_rect, frame.player->pos);
		sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
		sgp_draw_debug_rect_lines(interact_rect);
		ForEachFlat(entity, world->entities, Entity*) {
			if (!entity->interactable || entity->id == world->held_entity_id)
				continue;
			if (Overlap2F32(EntityBoundsInWorld(entity), interact_rect)) {
				frame.hovered_entity = entity;
			}
		}
		// todo - pull out hovered entity into ID that way it doesn't go stale past the interaction step and instantly clears?

		static Coroutine coro = { 0 };
		ProcessPlayerInteraction(&coro, &frame);
	}

	// ENTITY UPDATE
	ForEachFlat(entity, world->entities, Entity*)
	{
		if (entity->id == 0)
			continue;

		// timer to zero
		if (entity->zero_timer != 0)
		{
			entity->zero_timer -= delta_t;
			if (entity->zero_timer <= 0)
			{
				entity->zero_timer = 0;
			}
		}

		switch(entity->type)
		{
		case ENTITY_seed:
			CoroSeedUpdate(entity);
			break;
		}
	}

	// PLANT UPDATE
	ForEachFlat(plant, world->entities, Entity*) {
		if (plant->type != ENTITY_plant)
			continue;

		S8 previous_stage = plant->plant_stage;
		if (plant->zero_timer != 0)
		{
			plant->zero_timer -= delta_t;
			if (plant->zero_timer <= 0)
			{
				plant->plant_stage++;
				plant->zero_timer = 0;
				if (plant->plant_stage < plant_stage_max)
				{
					plant->zero_timer = plant_growth_speed; // kick off another timer
				}
			}
		}
		else if (plant->plant_stage == 0)
		{
			plant->zero_timer = plant_growth_speed;
		}

		Sprite* plant_sprite = th_texture_sprite_get("plant0");
		plant_sprite += plant->plant_stage;
		plant->sprite = plant_sprite;
		
		B8 at_max_growth = plant->plant_stage == plant_stage_max;

		if (plant->plant_stage != previous_stage && at_max_growth)
		{
			// @tooling - some kind of handle information from the sprite? maybe like a red pixel, or create another layer on information on top? Ideally I'd like to have another application running in the background where I can author this data.
			Entity* res_a = EntityCreateResource();
			res_a->pos = plant->pos;
			res_a->pos.y += 13;
			res_a->pos.x += -4;
			res_a->rigid_body = 0;
			EntityPushChild(plant, res_a->id);
			// PushChild could actually put a parent ID in?
			// just gotta be careful to keep them in sync via the helper funcs

			Entity* res_b = EntityCreateResource();
			res_b->pos = plant->pos;
			res_b->pos.y += 36;
			res_b->pos.x += 6;
			res_b->rigid_body = 0;
			EntityPushChild(plant, res_b->id);
		}

		if (at_max_growth && !EntityHasChildren(plant))
		{
			plant->interactable = 1;
		}
	}

	// Particle Emit
	ForEachFlat(emitter, world->entities, Entity*)
	{
		if (emitter->type != ENTITY_particle_emitter)
			continue;

		F32 freq_remainder = emitter->frequency - floorf(emitter->frequency);
		B8 remainder = 0;
		if (((F32)rand() / (F32)RAND_MAX) < freq_remainder)
			remainder = 1;

		U32 emit_amount = floorf(emitter->frequency) + remainder;
		for (int j = 0; j < emit_amount; j++) {
			gs->particle_count++;
			if (gs->particle_count == ArrayCount(gs->particles))
				gs->particle_count = 0;
			Particle* new_particle = &gs->particles[gs->particle_count];
			if (!float_is_zero(new_particle->life))
				LOG("warning: particles are being overridden");
			emitter->emit_func(new_particle, emitter);
		}
	}

	// Particle Render
	for (int i = 0; i < ArrayCount(gs->particles); i++) {
		Particle* particle = &gs->particles[i];
		if (float_is_zero(particle->life))
			continue;

		Assert(particle->life > 0.f);
		particle->life -= delta_t;
		if (particle->life <= 0.f) {
			MemoryZeroStruct(particle);
			continue;
		}

		particle->pos += particle->vel * delta_t;

		F32 alpha = float_alpha(particle->life, particle->start_life, 0.f);
		alpha = float_alpha_sin_mid(alpha);

		DeferLoop(sgp_push_transform(), sgp_pop_transform()) {
			Vec2 render_size = Vec2(1, 1) * particle->size_mult;
			sgp_set_color(particle->col.r, particle->col.g, particle->col.b, particle->col.a * alpha);
			sgp_translate(particle->pos.x, particle->pos.y);
			sgp_draw_filled_rect(render_size.x * -0.5f, render_size.y * -0.5f, render_size.x, render_size.y);
		}
	}

	sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
	sgp_draw_line(-200.f, 0.0f, 200.0f, 0.0f); // ground line

	// Entity Render
	ForEachFlat(entity, world->entities, Entity*) {
		if (!entity->render)
			continue;

		Rng2F32 rect = Shift2F32(entity->render_rect, entity->pos);
		rect = Shift2F32(rect, entity->render_offset);
		sgp_set_color(V4_EXPAND(entity->col));
		if (entity->frame.render_highlight)
			sgp_set_color(0.5f, 0.5f, 0.5f, 0.5f);
		if (entity->sprite) {
			Assert(entity->sprite->atlas); // invalid atlas
			sgp_rect target_rect = range2_to_sgp_rect(rect);
			sgp_rect src_rect = range2_to_sgp_rect(Pad2F32(entity->sprite->sub_rect, -0.1f)); // todo - fix this texture bleeding issue without padding it in
			if (entity->x_dir == -1) {
				target_rect.x += target_rect.w;
				target_rect.w *= -1;
			}
			sgp_set_image(0, entity->sprite->atlas->image);
			sgp_draw_textured_rect_ex(0, target_rect, src_rect);
			sgp_unset_image(0);
			sgp_reset_image(0);
		} else {
			sgp_rect icky_rect = range2_to_sgp_rect(rect);
			sgp_draw_filled_rect(icky_rect.x, icky_rect.y, icky_rect.w, icky_rect.h);
		}
	}

	#ifdef RENDER_COLLIDERS
	for (int i = 0; i < world->entities.count; i++) {
		Entity* entity = &world->entities[i];
		if (!entity->rigid_body)
			continue;
		Rng2F32 rect = entity->bounds;
		rect = Shift2F32(rect, entity->pos);
		sgp_set_color(RENDER_COLLIDER_COLOR);
		sgp_draw_debug_rect_lines(rect);
	}
	#endif

	sg_pass_action pass_action = { 0 };
	sg_begin_default_pass(&pass_action, window_size.x, window_size.y);
	sgp_flush();
	sgp_end();
	sg_end_pass();
	sg_commit();

	// clear press and release events
	memset(&gs->key_pressed, 0, sizeof(gs->key_pressed));
	memset(&gs->key_released, 0, sizeof(gs->key_released));
	memset(&gs->mouse_pressed, 0, sizeof(gs->mouse_pressed));
	memset(&gs->mouse_released, 0, sizeof(gs->mouse_released));

	// tick down entity lifetimes
	ForEachFlat(entity, world->entities, Entity*)
	{
		if (entity->lifetime_ticks_remaining == 0)
			continue;

		entity->lifetime_ticks_remaining--;
		if (entity->lifetime_ticks_remaining == 0)
		{
			EntityDestroy(entity);
		}
	}
}

static void init(void) {
	stm_setup();

	sg_desc sgdesc = { .context = sapp_sgcontext() };
	sg_setup(&sgdesc);
	if (!sg_isvalid()) {
		fprintf(stderr, "Failed to create Sokol GFX context!\n");
		exit(-1);
	}

	sgp_desc sgpdesc = { 0 };
	sgp_setup(&sgpdesc);
	if (!sgp_is_valid()) {
		fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error_message(sgp_get_last_error()));
		exit(-1);
	}

	// Allocate Game Memory
	#define PERMANENT_ARENA_SIZE Megabytes(32)
	#define WORLD_ARENA_SIZE Megabytes(32)
	#define FRAME_ARENA_SIZE Megabytes(32)

	size_t game_memory_size = PERMANENT_ARENA_SIZE + WORLD_ARENA_SIZE + FRAME_ARENA_SIZE * 2;
	U8* base_memory = (U8*)malloc(game_memory_size);
	U8* memory = base_memory;
	MemoryZero(memory, game_memory_size);

	M_ArenaInit(&game_memory.permanent_arena, memory, PERMANENT_ARENA_SIZE);
	memory += PERMANENT_ARENA_SIZE;

	M_ArenaInit(&game_memory.world_arena, memory, WORLD_ARENA_SIZE);
	memory += WORLD_ARENA_SIZE;

	M_ArenaInit(&game_memory.frame_arena, memory, FRAME_ARENA_SIZE);
	memory += FRAME_ARENA_SIZE;

	M_ArenaInit(&game_memory.last_frame_arena, memory, FRAME_ARENA_SIZE);
	memory += FRAME_ARENA_SIZE;

	Assert(memory - base_memory == game_memory_size);

	GameState* gs = game_state();
	WorldState* world = world_state();

	// todo - allocate this on the arenas, not the globals.


	// TEXTURES
	TextureAtlas* atlas = th_texture_atlas_load("dump.png");
	// plant stages
	Rng2F32 sub_rect = Rng2F32(Vec2(), Vec2(16, 64));
	th_texture_sprite_create(atlas, "plant0", sub_rect);
	sub_rect = Shift2F32(sub_rect, Vec2(16, 0));
	th_texture_sprite_create(atlas, "plant1", sub_rect);
	sub_rect = Shift2F32(sub_rect, Vec2(16, 0));
	th_texture_sprite_create(atlas, "plant2", sub_rect);
	sub_rect = Shift2F32(sub_rect, Vec2(16, 0));
	th_texture_sprite_create(atlas, "plant3", sub_rect);
	sub_rect = Shift2F32(sub_rect, Vec2(16, 0));
	th_texture_sprite_create(atlas, "plant4", sub_rect);
	sub_rect = Shift2F32(sub_rect, Vec2(16, 0));
	th_texture_sprite_create(atlas, "plant5", sub_rect);
	sub_rect = Shift2F32(sub_rect, Vec2(16, 0));
	th_texture_sprite_create(atlas, "plant6", sub_rect);
	sub_rect = Shift2F32(sub_rect, Vec2(16, 0));
	th_texture_sprite_create(atlas, "plant7", sub_rect);
	// resources
	sub_rect = Shift2F32(sub_rect, Vec2(16, 0));
	sub_rect.max = sub_rect.min;
	sub_rect.max += Vec2(4, 4);
	th_texture_sprite_create(atlas, "resource1", sub_rect);
	// player
	sub_rect.min = Vec2(16 * 10, 0);
	sub_rect.max = sub_rect.min + Vec2(16, 32);
	th_texture_sprite_create(atlas, "arcane_player", sub_rect);

	{
		// background emitter
		Entity* emit = EntityCreateEmitter();
		emit->emit_func = emitter_ambient_screen;
		emit->frequency = 10.0f;
	}

	gs->cam.scale = DEFAULT_CAMERA_SCALE;

	WorldInit(world);
}

static void cleanup(void) {
	sgp_shutdown();
	sg_shutdown();
	free(game_memory.permanent_arena.buffer);
}

static void event(const sapp_event* ev) {
	GameState* gs = game_state();
	switch (ev->type) {
	case SAPP_EVENTTYPE_KEY_UP: {
		B8 already_up = !gs->key_down[ev->key_code];
		if (!already_up)
			gs->key_released[ev->key_code] = 1;
		gs->key_down[ev->key_code] = 0;
	} break;
	case SAPP_EVENTTYPE_KEY_DOWN: {
		B8 already_down = gs->key_down[ev->key_code];
		if (!already_down)
			gs->key_pressed[ev->key_code] = 1;
		gs->key_down[ev->key_code] = 1;
	} break;
	case SAPP_EVENTTYPE_MOUSE_UP: {
		B8 already_up = !gs->mouse_down[ev->mouse_button];
		if (!already_up)
			gs->mouse_released[ev->mouse_button] = 1;
		gs->mouse_down[ev->mouse_button] = 0;
	} break;
	case SAPP_EVENTTYPE_MOUSE_DOWN: {
		B8 already_down = gs->mouse_down[ev->mouse_button];
		if (!already_down)
			gs->mouse_pressed[ev->mouse_button] = 1;
		gs->mouse_down[ev->mouse_button] = 1;
	} break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL: {
		gs->cam.scale += ev->scroll_y / 8.0f;
		gs->cam.scale = Clamp(1.0f, gs->cam.scale, 10.0f);
	} break;
	case SAPP_EVENTTYPE_MOUSE_MOVE: {
		gs->mouse_pos = Vec2(ev->mouse_x, ev->mouse_y);
	};
	}

	#ifdef FUN_VAL
	fun_val = float_map(ev->mouse_y, 0, gs->window_size.y, -10.f, 10.f);
	fun_val *= float_map(ev->mouse_x, 0, gs->window_size.x, 0.00001f, 100.f);
	LOG("%f", fun_val);
	#endif
}

sapp_desc sokol_main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	sapp_desc test = {
		.init_cb = init,
		.frame_cb = frame,
		.cleanup_cb = cleanup,
		.event_cb = event,
		.sample_count = 2,
		.window_title = APP_NAME,
		.win32_console_attach = 1,
	};
	return test;
}