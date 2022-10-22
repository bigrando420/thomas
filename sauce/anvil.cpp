#define SOKOL_D3D11
#define SOKOL_IMPL
#include "ext/sokol_gfx.h"
#include "ext/sokol_gp.h"
#include "ext/sokol_app.h"
#include "ext/sokol_glue.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include "ext/HandmadeMath.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "thomas.h"
#include "anvil.h"

static void frame(void) {
	GameState* gs = game_state();
	WorldState* world = world_state();
	Entity*& player = world->player;
	const vec2 window_size = { (float)sapp_width(), (float)sapp_height() };
	gs->window_size = window_size;
	float ratio = window_size.x / (float)window_size.y;
	float delta_t = sapp_frame_duration();
	const vec2 world_mouse = mouse_pos_in_worldspace();

	#pragma region Player Input
	if (gs->key_pressed[SAPP_KEYCODE_SPACE]) {
		player->vel.y = 300.0f;
	}
	vec2 axis_input = { 0 };
	if (gs->key_down[SAPP_KEYCODE_A]) {
		axis_input.x -= 1.0f;
		world->player->flip_horizontal = 1;
	}
	if (gs->key_down[SAPP_KEYCODE_D]) {
		axis_input.x += 1.0f;
		world->player->flip_horizontal = 0;
	}
	player->acc = axis_input * MOVE_SPEED;
	#pragma endregion

	// Particle Emit
	for (int i = 0; i < gs->emitters.count; i++) {
		Emitter* emitter = &gs->emitters[i];
		
		float freq_remainder = emitter->frequency - floorf(emitter->frequency);
		bool8 remainder = 0;
		if (((float)rand() / (float)RAND_MAX) < freq_remainder)
			remainder = 1;

		uint32 emit_amount = floorf(emitter->frequency) + remainder;
		for (int j = 0; j < emit_amount; j++) {
			gs->particles.count++;
			if (gs->particles.count == gs->particles.max_count)
				gs->particles.count = 0;
			Particle* new_particle = &gs->particles[gs->particles.count];
			if (!float_is_zero(new_particle->life))
				LOG("warning: particles are being overridden");
			emitter->emit_func(new_particle, emitter);
		}
	}

	// Entity Physics
	for (int i = 0; i < world->entities.count; i++) {
		Entity* entity = &world->entities[i];
		if (!entity->rigid_body)
			continue;

		// acc counter force with existing velocity
		entity->acc.x += -15.0f * entity->vel.x;

		// gravity
		bool8 falling = entity->vel.y < 0.f;
		entity->acc.y -= (falling ? 2.f : 1.f) * GRAVITY;

		// integrate acceleration and velocity into position
		vec2 next_pos = 0.5f * entity->acc * SQUARE(delta_t) + entity->vel * delta_t + entity->pos;

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

	// update camera
	gs->cam.pos.x = player->pos.x;
	gs->cam.pos.y = 20.0f;

	// BEGIN RENDER
	sgp_begin(window_size.x, window_size.y);
	sgp_viewport(0, 0, window_size.x, window_size.y);

	#if 1
	sgp_project(window_size.x * -0.5f, window_size.x * 0.5f, window_size.y * 0.5f, window_size.y * -0.5f);
	sgp_scale(gs->cam.scale, gs->cam.scale);
	sgp_translate(-gs->cam.pos.x, -gs->cam.pos.y);
	#else
	range2 view_rect = { 0 };
	view_rect.max = vec2(window_size.x, window_size.y);
	view_rect = range2_center_middle(view_rect);
	view_rect = range2_shift(view_rect, vec2(0, 100.f));
	view_rect = range2_scale(view_rect, gs->cam.scale);
	view_rect = range2_shift(view_rect, gs->cam.pos);
	sgp_project(view_rect.min.x, view_rect.max.x, view_rect.max.y, view_rect.min.y);
	#endif

	// @sgp_helpers - vector expander, so I can use my types
	sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
	sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
	sgp_clear();

	// SEED
	if (world->held_seed) {
		Entity* seed = world->held_seed;
		seed->pos.x = roundf(world_mouse.x);
		seed->pos.y = 0.0f;

		sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
		sgp_draw_line(seed->pos.x, world_mouse.y, seed->pos.x, 0.0f);
	}

	// Particle Render
	for (int i = 0; i < gs->particles.max_count; i++) {
		Particle* particle = &gs->particles[i];
		if (float_is_zero(particle->life))
			continue;

		assert(particle->life > 0.f);
		particle->life -= delta_t;
		if (particle->life <= 0.f) {
			MEMORY_ZERO_STRUCT(particle);
			continue;
		}

		particle->pos += particle->vel * delta_t;

		float alpha = float_alpha(particle->life, particle->start_life, 0.f);
		alpha = float_alpha_sin_mid(alpha);

		DEFER_LOOP(sgp_push_transform(), sgp_pop_transform()) {
			vec2 render_size = vec2(1, 1) * particle->size_mult;
			sgp_set_color(particle->col.r, particle->col.g, particle->col.b, particle->col.a * alpha);
			sgp_translate(particle->pos.x, particle->pos.y);
			sgp_draw_filled_rect(render_size.x * -0.5f, render_size.y * -0.5f, render_size.x, render_size.y);
		}
	}

	sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
	sgp_draw_line(-200.f, 0.0f, 200.0f, 0.0f);

	// Entity Render
	for (int i = 0; i < world->entities.count; i++) {
		Entity* entity = &world->entities[i];
		if (!entity->render)
			continue;

		for (int j = 0; j < entity->render_rects.count; j++) {
			RenderRect* render = &entity->render_rects[j];
			
			range2 rect = render->rect;
			if (entity->flip_horizontal) {
				rect.min.x *= -1.0f;
				rect.max.x *= -1.0f;
				float temp = rect.min.x;
				rect.min.x = rect.max.x;
				rect.max.x = temp;
			}
			rect = range2_shift(rect, entity->pos);

			// draw rect
			vec2 pos = rect.min;
			vec2 size = range2_size(rect);
			sgp_set_color(V4_EXPAND(render->col));
			sgp_draw_filled_rect(pos.x, pos.y, size.x, size.y);
		}
	}

	#ifdef RENDER_COLLIDERS
	for (int i = 0; i < world->entities.count; i++) {
		Entity* entity = &world->entities[i];
		if (!entity->rigid_body)
			continue;
		range2 rect = entity->bounds;
		rect = range2_shift(rect, entity->pos);
		sgp_set_color(RENDER_COLLIDER_COLOR);
		sgp_draw_line(rect.min.x, rect.min.y, rect.min.x, rect.max.y);
		sgp_draw_line(rect.min.x, rect.min.y, rect.max.x, rect.min.y);
		sgp_draw_line(rect.max.x, rect.max.y, rect.min.x, rect.max.y);
		sgp_draw_line(rect.max.x, rect.max.y, rect.max.x, rect.min.y);
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
	memset(&gs->key_released, 0, sizeof(gs->key_pressed));
}

static void init(void) {
	GameState* gs = game_state();
	WorldState* world = world_state();
	{
		// player
		Entity* entity = world->entities.push();
		world->player = entity;
		entity->bounds.max = vec2(5.0f, 10.0f);
		entity->bounds = range2_center_bottom(entity->bounds);
		entity->pos.y = 100.0f;
		entity->rigid_body = 1;
		entity->render = 1;
		entity_render_rect_from_bounds(entity, TH_WHITE);
		RenderRect* eye = entity->render_rects.push();
		eye->col = TH_BLACK;
		eye->rect.max = vec2(1, 1);
		eye->rect = range2_shift(eye->rect, vec2(1.0f, 8.0f));
	}
	{
		// test seed
		Entity* entity = world->entities.push();
		entity->bounds.max = vec2(2.0f, 2.0f);
		entity->bounds = range2_center_bottom(entity->bounds);
		entity->render = 1;
		//entity->rigid_body = 1;
		entity_render_rect_from_bounds(entity, TH_WHITE);
		world->held_seed = entity;
	}
	{
		// background emitter
		Emitter* emitter = gs->emitters.push();
		emitter->emit_func = emitter_ambient_screen;
		emitter->frequency = 10.0f;
	}
	gs->cam.scale = DEFAULT_CAMERA_SCALE;

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
}

static void cleanup(void) {
	sgp_shutdown();
	sg_shutdown();
}

static void event(const sapp_event* ev) {
	GameState* gs = game_state();
	switch (ev->type) {
	case SAPP_EVENTTYPE_KEY_UP: {
		bool8 already_up = !gs->key_down[ev->key_code];
		if (!already_up)
			gs->key_released[ev->key_code] = 1;
		gs->key_down[ev->key_code] = 0;
	} break;
	case SAPP_EVENTTYPE_KEY_DOWN: {
		bool8 already_down = gs->key_down[ev->key_code];
		if (!already_down)
			gs->key_pressed[ev->key_code] = 1;
		gs->key_down[ev->key_code] = 1;
	} break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL: {
		gs->cam.scale += ev->scroll_y / 8.0f;
		gs->cam.scale = CLAMP(1.0f, gs->cam.scale, 10.0f);
	} break;
	case SAPP_EVENTTYPE_MOUSE_MOVE: {
		gs->mouse_pos = vec2(ev->mouse_x, ev->mouse_y);
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
	};
	return test;
}