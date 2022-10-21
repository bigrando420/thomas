#ifndef ANVIL_H
#define ANVIL_H

#define APP_NAME "Game C"
#define MOVE_SPEED 2000.0f
#define PIXEL_SCALE 30.0f
#define GRAVITY 1000.0f
#define DEFAULT_CAMERA_SCALE 1.0f

#ifndef TH_SHIP
//#define FUN_VAL
#endif

typedef struct Emitter Emitter;
typedef struct Particle Particle;
typedef void (*ParticleEmitterFunc)(Particle*, Emitter*);
#define PARTICLE_EMITTER_FUNC(function_name) static void function_name(Particle* particle, Emitter* emitter)

struct Emitter
{
	vec2 pos;
	float frequency;
	// frequency is amount of particles to be spawned each frame. The decimal is the % chance of it rounding up: 0.2 == 20% chance to spawn each frame
	ParticleEmitterFunc emit_func;
};

struct Particle
{
	vec2 pos;
	vec2 vel;
	vec4 col;
	float start_life;
	float life;
	float size_mult;

	// flags;
	bool8 fade_in;
	bool8 fade_out;
};

// An ECS made simple. The Megastruct.
struct Entity {
	vec2 pos;
	vec2 vel;
	vec2 acc;
	range2 bounds;
	// flags for which "components" are active
	bool rigid_body;
	bool render;
};

struct Camera {
	vec2 pos;
	float scale;
	float rotation;
};

struct WorldState {
	array_flat<Entity, 64> entities;
	Entity* player;
	Entity* held_seed;
};

struct GameState {
	WorldState world_state;
	bool8 key_down[SAPP_KEYCODE_MENU];
	array_flat<Emitter, 16> emitters;
	array_flat<Particle, 2048> particles;
	Camera cam;
	// per-frame
	vec2 mouse_pos;
	vec2 window_size;
	bool8 key_pressed[SAPP_KEYCODE_MENU];
	bool8 key_released[SAPP_KEYCODE_MENU];
};

// wrapped globals, will be trivial to swap out later
static GameState* game_state() {
	static GameState gs = { 0 };
	return &gs;
}
static WorldState* world_state() {
	GameState* gs = game_state();
	return &gs->world_state;
}

#ifdef FUN_VAL
static float fun_val = 0.0f;
#endif

static range2 camera_get_bounds() {
	GameState* gs = game_state();
	// todo - @camera
	range2 cam = { 0 };
	cam.max = vec2((int)gs->window_size.x, (int)gs->window_size.y);
	cam = range2_shift(cam, vec2((int)gs->window_size.x, (int)gs->window_size.y) * -0.5f);
	return cam;
}

PARTICLE_EMITTER_FUNC(emitter_ambient_screen) {
	range2 bounds = camera_get_bounds();
	particle->pos.x = float_random_range(bounds.min.x, bounds.max.x);
	particle->pos.y = float_random_range(bounds.min.y, bounds.max.y);
	particle->vel = vec2(float_random_range(-1.f, 1.f), float_random_range(2.f, 4.f));
	particle->col = vec4(0.7f, 0.7f, 0.7f, 1.0f);
	particle->start_life = particle->life = 2.f;
	particle->fade_in = 1;
	particle->fade_out = 1;
	particle->size_mult = 1.f;
}

// ideally this is just the inverse of the projection matrix, but I don't have the paitence
// @camera - construct my own 2x3 camera view matrix and pass it down to sokol_gp somehow by pushing it as a transform?
// or maybe just push n pop properly? idk
static vec2 world_pos_to_screen_pos(const vec2& world_pos, const Camera& cam) {
	GameState* gs = game_state();
	vec2 result = world_pos;
	result = result - cam.pos;
	result.x *= cam.scale;
	result.y *= cam.scale;
	result += gs->window_size * 0.5f;
	result.y = gs->window_size.y - result.y;
	return result;
}

static vec2 screen_pos_to_world_pos(const vec2& screen_pos, const Camera& cam) {
	GameState* gs = game_state();
	vec2 result = screen_pos;
	result.y = gs->window_size.y - result.y;
	result += gs->window_size * -0.5f;
	result.x /= cam.scale;
	result.y /= cam.scale;
	result += cam.pos;
	return result;
}

static vec2 mouse_pos_in_worldspace() {
	GameState* gs = game_state();
	return screen_pos_to_world_pos(gs->mouse_pos, gs->cam);
}

#endif