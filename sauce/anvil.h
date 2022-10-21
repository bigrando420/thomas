#ifndef ANVIL_H
#define ANVIL_H

#define APP_NAME "Game C"
#define MOVE_SPEED 2000.0f
#define PIXEL_SCALE 30.0f
#define GRAVITY 1000.0f

#ifndef TH_SHIP
#define FUN_VAL
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

struct RigidBody {
	range2 box;
	vec2 pos;
	vec2 vel;
	vec2 acc;
};

struct Entity {
	RigidBody body;
	// flags (does c++ auto pack these bools?)
	bool rigid_body;
	bool is_player;
};

struct Camera {
	vec2 pos;
	float scale;
	float rotation;
};

struct WorldState {
	array_flat<Entity, 64> entities;
	Entity* player;
};

struct GameState {
	WorldState world_state;
	bool8 key_down[SAPP_KEYCODE_MENU];
	array_flat<Emitter, 16> emitters;
	array_flat<Particle, 2048> particles;
	Camera cam;
	// per-frame
	vec2i window_size;
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

static void camera_apply_transform(const Camera& cam) {
	sgp_translate(-cam.pos.x, -cam.pos.y);
}

#endif