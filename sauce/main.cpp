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

#include "base.h"

/*

## Design Philosophy
- Keep it (C)imple.
- Speed is king.

### Why C++ over of C ?
- operator overloading for common math ops
- simple templated generics for Array<> containers
- that's literally it.

## Dump

figure out how I wanna get this array goin
template <typename T> struct List
{
	T* first;
};

*/

#define APP_NAME "Game C"
#define MOVE_SPEED 100.0f
#define PIXEL_SCALE 30.0f

template <typename T, uint32 max_count>
struct array_flat {
	uint32 count;
	T elements[max_count];

	T& operator[](uint32 index) {
		assert(index >= 0 && index < max_count);
		return elements[index];
	}

	// self mutating? that way all the templatey stuff is contained?
	// .push
};

struct RigidBody {
	range2 box;
	vec2 pos;
	vec2 vel;
	vec2 acc;
};

struct WorldState {
	array_flat<RigidBody, 128> rigid_bodies;
};

struct AppState {
	WorldState world;
};
global AppState state = {0};

function void frame(void) {
	vec2i view_rect = { sapp_width(), sapp_height() };
	real ratio = view_rect.x / (real)view_rect.y;
	real delta_t = sapp_frame_duration();

	// BEGIN RENDER
	sgp_begin(view_rect.x, view_rect.y);
	sgp_viewport(0, 0, view_rect.x, view_rect.y);
	sgp_project(view_rect.x * -0.5f / PIXEL_SCALE, view_rect.x * 0.5f / PIXEL_SCALE, view_rect.y * 0.5f / PIXEL_SCALE, view_rect.y * -0.5f / PIXEL_SCALE);

	sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
	sgp_clear();

	// Physics update
	for (int i = 0; i < state.world.rigid_bodies.count; i++)
	{
		RigidBody* body = &state.world.rigid_bodies[i];
		body->acc = vec2(-1.f, -1.f) * MOVE_SPEED;

		// acc counter force with existing velocity
		body->acc.x += -15.0f * body->vel.x;

		// gravity
		bool8 falling = body->vel.y < 0.f;
		body->acc.y -= (falling ? 2.f : 1.f) * 100.f;

		// integrate acceleration and velocity into position
		vec2 next_pos = 0.5f * body->acc * SQUARE(delta_t) + body->vel * delta_t + body->pos;

		// integrate acceleration into velocity
		body->vel += body->acc * delta_t;
		body->acc.x = 0;
		body->acc.y = 0;

		if (next_pos.y < 0.f)
		{
			next_pos.y = 0.f;
		}

		body->pos = next_pos;

		// push render rect
		DEFER_LOOP(sgp_push_transform(), sgp_pop_transform())
		{
			range2 rect = body->box;
			rect = range2_shift(rect, body->pos);

			sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
			sgp_scale(0.1f, 0.1f);
			sgp_translate(body->pos.x, body->pos.y);
			sgp_draw_filled_rect(rect.min.x, rect.min.y, rect.max.x - rect.min.x, rect.max.y - rect.min.y);
		}
	}

	real time = sapp_frame_count() * sapp_frame_duration();
	real r = sinf(time)*0.5 + 0.5, g = cosf(time)*0.5 + 0.5;
	//sgp_set_color(r, g, 0.3f, 1.0f);
	//sgp_rotate_at(time, 0.0f, 0.0f);
	//sgp_scale(100.0f, 100.f);
	//sgp_draw_filled_rect(-0.5f, -0.5f, 1.0f, 1.0f);

	sg_pass_action pass_action = { 0 };
	sg_begin_default_pass(&pass_action, view_rect.x, view_rect.y);
	sgp_flush();
	sgp_end();
	sg_end_pass();
	sg_commit();
}

function void init(void) {

	state.world.rigid_bodies[0].box = range2(vec2(0, 0), vec2(5.0f, 10.0f));
	state.world.rigid_bodies[0].pos.y = 100.0f;
	state.world.rigid_bodies.count++;

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

function void cleanup(void) {
	sgp_shutdown();
	sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	sapp_desc test = {
		.init_cb = init,
		.frame_cb = frame,
		.cleanup_cb = cleanup,
		.sample_count = 4, // Enable anti aliasing.
		.window_title = APP_NAME,
	};
	return test;
}