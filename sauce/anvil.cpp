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

/*

## Design Philosophy
- Keep it (C)imple.
- Speed is king.

### Why C++ over of C ?
- operator overloading for common math ops
- simple templated generics for array<> containers
- that's literally it.

## Dump
- [ ] basic memory arenas that don't make use of 64-bit so I can be portable with WASM

*/

#include "thomas.h"
#include "anvil.h"

static void frame(void) {
	const vec2i window_size = { sapp_width(), sapp_height() };
	state->window_size = window_size;
	float ratio = window_size.x / (float)window_size.y;
	float delta_t = sapp_frame_duration();
	RigidBody* player = world->player;

	#pragma region Player Input
	if (state->key_pressed[SAPP_KEYCODE_SPACE]) {
		player->vel.y = 300.0f;
	}
	vec2 axis_input = { 0 };
	if (state->key_down[SAPP_KEYCODE_A]) {
		axis_input.x -= 1.0f;
	}
	if (state->key_down[SAPP_KEYCODE_D]) {
		axis_input.x += 1.0f;
	}
	player->acc = axis_input * MOVE_SPEED;
	#pragma endregion

	// BEGIN RENDER
	sgp_begin(window_size.x, window_size.y);
	sgp_viewport(0, 0, window_size.x, window_size.y);
	
	float project_scale = state->cam.scale;
	range2 render_resolution = range2(vec2(window_size.x * -0.5f * project_scale, window_size.y * -0.5f * project_scale), vec2(window_size.x * 0.5f * project_scale, window_size.y * 0.5f * project_scale));
	sgp_project(render_resolution.min.x, render_resolution.max.x, render_resolution.max.y, render_resolution.min.y);

	// @sgp_helpers - vector expander, so I can use my types
	sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
	sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
	sgp_clear();

	#pragma region Particle System

	for (int i = 0; i < state->emitters.count; i++) {
		Emitter* emitter = &state->emitters[i];
		
		float freq_remainder = emitter->frequency - floorf(emitter->frequency);
		bool8 remainder = 0;
		if (((float)rand() / (float)RAND_MAX) < freq_remainder)
			remainder = 1;

		uint32 emit_amount = floorf(emitter->frequency) + remainder;
		for (int j = 0; j < emit_amount; j++) {
			state->particles.count++;
			if (state->particles.count == state->particles.max_count)
				state->particles.count = 0;
			Particle* new_particle = &state->particles[state->particles.count];
			if (!float_is_zero(new_particle->life))
				LOG("warning: particles are being overridden");
			emitter->emit_func(new_particle, emitter);
		}
	}

	for (int i = 0; i < state->particles.max_count; i++) {
		Particle* particle = &state->particles[i];
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
			camera_apply_transform(state->cam);

			sgp_draw_filled_rect(render_size.x * -0.5f, render_size.y * -0.5f, render_size.x * 0.5f, render_size.y * 0.5f);
		}
	}

	#pragma endregion

	// Physics update
	for (int i = 0; i < world->rigid_bodies.count; i++) {
		RigidBody* body = &world->rigid_bodies[i];

		// acc counter force with existing velocity
		body->acc.x += -15.0f * body->vel.x;

		// gravity
		bool8 falling = body->vel.y < 0.f;
		body->acc.y -= (falling ? 2.f : 1.f) * GRAVITY;

		// integrate acceleration and velocity into position
		vec2 next_pos = 0.5f * body->acc * SQUARE(delta_t) + body->vel * delta_t + body->pos;

		// integrate acceleration into velocity
		body->vel += body->acc * delta_t;
		body->acc.x = 0;
		body->acc.y = 0;

		if (next_pos.y < 0.f) {
			next_pos.y = 0.f;
		}

		body->pos = next_pos;
	}

	// update camera
	state->cam.pos.x = player->pos.x;

	// render rigid bodies
	for (int i = 0; i < world->rigid_bodies.count; i++) {
		RigidBody* body = &world->rigid_bodies[i];

		// @sgp_helpers - transform scope
		DEFER_LOOP(sgp_push_transform(), sgp_pop_transform()) {
			range2 rect = body->box;
			vec2 render_size = range2_size(rect);

			sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);

			sgp_translate(body->pos.x, body->pos.y);
			camera_apply_transform(state->cam);

			sgp_draw_filled_rect(rect.min.x, rect.min.y, render_size.x, render_size.y);
			// @sgp_helpers - function for pushing a rang2 rect
		}
	}

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
	memset(&state->key_pressed, 0, sizeof(state->key_pressed));
	memset(&state->key_released, 0, sizeof(state->key_pressed));
}

static void init(void) {
	RigidBody player = { 0 };
	player.box = range2(vec2(0, 0), vec2(5.0f, 10.0f));
	player.pos.y = 100.0f;
	state->world.player = world->rigid_bodies.push(player);

	Emitter emitter = { 0 };
	emitter.emit_func = emitter_ambient_screen;
	emitter.frequency = 10.0f;
	state->emitters.push(emitter);

	state->cam.scale = 0.2f;

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
	switch (ev->type) {
	case SAPP_EVENTTYPE_KEY_UP: {
		bool8 already_up = !state->key_down[ev->key_code];
		if (!already_up)
			state->key_released[ev->key_code] = 1;
		state->key_down[ev->key_code] = 0;
	} break;
	case SAPP_EVENTTYPE_KEY_DOWN: {
		bool8 already_down = state->key_down[ev->key_code];
		if (!already_down)
			state->key_pressed[ev->key_code] = 1;
		state->key_down[ev->key_code] = 1;
	} break;
	}

	fun_val = float_map(ev->mouse_y, 0, state->window_size.y, -10.f, 10.f);
	fun_val *= float_map(ev->mouse_x, 0, state->window_size.x, 0.00001f, 100.f);
	LOG("%f", fun_val);
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