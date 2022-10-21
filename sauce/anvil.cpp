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
	const vec2i window_size = { sapp_width(), sapp_height() };
	gs->window_size = window_size;
	float ratio = window_size.x / (float)window_size.y;
	float delta_t = sapp_frame_duration();
	
	#pragma region Player Input
	if (gs->key_pressed[SAPP_KEYCODE_SPACE]) {
		player->vel.y = 300.0f;
	}
	vec2 axis_input = { 0 };
	if (gs->key_down[SAPP_KEYCODE_A]) {
		axis_input.x -= 1.0f;
	}
	if (gs->key_down[SAPP_KEYCODE_D]) {
		axis_input.x += 1.0f;
	}
	player->acc = axis_input * MOVE_SPEED;
	#pragma endregion

	// BEGIN RENDER
	sgp_begin(window_size.x, window_size.y);
	sgp_viewport(0, 0, window_size.x, window_size.y);
	
	float project_scale = gs->cam.scale;
	range2 render_resolution = range2(vec2(window_size.x * -0.5f * project_scale, window_size.y * -0.5f * project_scale), vec2(window_size.x * 0.5f * project_scale, window_size.y * 0.5f * project_scale));
	sgp_project(render_resolution.min.x, render_resolution.max.x, render_resolution.max.y, render_resolution.min.y);

	// @sgp_helpers - vector expander, so I can use my types
	sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
	sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
	sgp_clear();

	#pragma region Particle System

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

	for (int i = 0; i < gs->particles.max_count; i++) {
		Particle* particle = &gs->particles[i];
		if (float_is_zero(particle->life))
			continue;
		
		assert(particle->life > 0.f);
		particle->life -= delta_t;
		if (particle->life <= 0.f)
		{
			MEMORY_ZERO_STRUCT(particle);
			continue;
		}

		particle->pos += particle->vel * delta_t;

		vec2 render_size = vec2(1, 1);

		float alpha = float_alpha(particle->life, particle->start_life, 0.f);
		alpha = float_alpha_sin_mid(alpha);

		DEFER_LOOP(sgp_push_transform(), sgp_pop_transform()) {
			sgp_set_color(particle->col.r, particle->col.g, particle->col.b, particle->col.a * alpha);
			
			sgp_translate(particle->pos.x, particle->pos.y);
			camera_apply_transform(gs->cam);

			sgp_draw_filled_rect(render_size.x * -0.5f, render_size.y * -0.5f, render_size.x * 0.5f, render_size.y * 0.5f);
		}
	}

	#pragma endregion

	// Physics update
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

		if (next_pos.y < 0.f) {
			next_pos.y = 0.f;
		}

		entity->pos = next_pos;
	}

	// update camera
	gs->cam.pos.x = player->pos.x;
	gs->cam.pos.y = 20.0f;

	sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
	sgp_draw_line(-200.f, -20.0f, 200.0f, -20.0f);

	#pragma region Render Entities
	for (int i = 0; i < world->entities.count; i++) {
		Entity* entity = &world->entities[i];
		if (!entity->render)
			continue;

		// @sgp_helpers - transform scope
		DEFER_LOOP(sgp_push_transform(), sgp_pop_transform()) {
			range2 rect = entity->bounds;
			vec2 render_size = range2_size(rect);

			sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);

			sgp_translate(entity->pos.x, entity->pos.y);
			camera_apply_transform(gs->cam);

			sgp_draw_filled_rect(rect.min.x, rect.min.y, render_size.x, render_size.y);
			// @sgp_helpers - function for pushing a rang2 rect
		}
	}
	#pragma endregion

	float time = sapp_frame_count() * sapp_frame_duration();
	float r = sinf(time)*0.5 + 0.5, g = cosf(time)*0.5 + 0.5;
	//sgp_set_color(r, g, 0.3f, 1.0f);
	//sgp_rotate_at(time, 0.0f, 0.0f);
	//sgp_scale(100.0f, 100.f);
	//sgp_draw_filled_rect(-0.5f, -0.5f, 1.0f, 1.0f);

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
		entity->bounds.max = vec2(5.0f, 10.0f);
		entity->bounds = range2_center_bottom(entity->bounds);
		entity->pos.y = 100.0f;
		entity->rigid_body = 1;
		entity->render = 1;
		entity->player = 1;
		world->player = entity;
	}

	{
		// test seed
		Entity* entity = world->entities.push();
		entity->pos = vec2(10.0f, 10.0f);
		entity->bounds.max = vec2(2.0f, 2.0f);
		entity->bounds = range2_center_middle(entity->bounds);
		entity->render = 1;
		entity->rigid_body = 1;
	}

	Emitter* emitter = gs->emitters.push();
	emitter->emit_func = emitter_ambient_screen;
	emitter->frequency = 10.0f;

	gs->cam.scale = 0.2f;

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
		gs->cam.scale -= ev->scroll_y / 40.0f;
		gs->cam.scale = CLAMP(0.1f, gs->cam.scale, 1.0f);
	} break;
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
		.sample_count = 4, // Enable anti aliasing.
		.window_title = APP_NAME,
	};
	return test;
}