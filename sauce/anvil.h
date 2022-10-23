#ifndef ANVIL_H
#define ANVIL_H

#define APP_NAME "Game C"
#define MOVE_SPEED 2000.0f
#define PIXEL_SCALE 30.0f
#define GRAVITY 1000.0f
#define DEFAULT_CAMERA_SCALE 1.0f

#ifndef TH_SHIP
//#define FUN_VAL
//#define RENDER_COLLIDERS
#define RENDER_COLLIDER_COLOR 1.0f, 0.5f, 0.5f, 1.0f
#endif

// helpers
#define TH_BLACK vec4(0.0f, 0.0f, 0.0f, 1.0f)
#define TH_WHITE vec4(1.0f, 1.0f, 1.0f, 1.0f)

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

struct Atlas {
	char name[128];
	sg_image image;
};

struct Sprite {
	char name[128];
	Atlas* atlas;
	range2 sub_rect;
};

struct RenderRect {
	range2 rect;
	Sprite* sprite;
	vec4 col;
};

// An ECS made simple. The Megastruct.
struct Entity {
	vec2 pos;
	vec2 vel;
	vec2 acc;
	range2 bounds;
	RenderRect render_rects[8];
	uint32 render_rect_count;
	bool8 flip_horizontal;
	float plant_stage;
	bool rigid_body;
	bool render;
	bool plant;
};

struct Camera {
	vec2 pos;
	float scale;
	float rotation;
};

struct WorldState {
	Entity entities[64];
	uint32 entity_count;
	Entity* player;
	Entity* held_seed;
};

struct GameState {
	WorldState world_state;
	bool8 key_down[SAPP_KEYCODE_MENU];
	bool8 mouse_down[SAPP_MOUSEBUTTON_MIDDLE];
	Emitter emitters[16];
	uint32 emitter_count;
	Particle particles[2048];
	uint32 particle_count;
	Camera cam;
	Atlas atlases[16];
	uint32 atlas_count;
	Sprite sprites[64];
	uint32 sprite_count;
	// per-frame
	vec2 mouse_pos;
	vec2 window_size;
	bool8 key_pressed[SAPP_KEYCODE_MENU];
	bool8 key_released[SAPP_KEYCODE_MENU];
	bool8 mouse_pressed[SAPP_MOUSEBUTTON_MIDDLE];
	bool8 mouse_released[SAPP_MOUSEBUTTON_MIDDLE];
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

static uint32 c_string_length(const char* string) {
	const char* cursor = string;
	while (cursor[0] != '\0') {
		cursor++;
	}
	uint32 length = cursor - string;
	return length;
}

static uint32 hash_from_string(const char* string) {
	uint32 result = 5381;
	uint32 string_size = c_string_length(string);
	for (int i = 0; i < string_size; i++) {
		result = ((result << 5) + result) + string[i];
	}
	return result;
}

static Atlas* th_image_from_string(const char* string) {
	GameState* gs = game_state();
	for (int i = 0; i < gs->atlas_count; i++) {
		Atlas* texture = &gs->atlases[i];
		if (strcmp(texture->name, string) == 0) {
			return texture;
		}
	}
	Assert(0); // no texture found :(
	return 0;
}

static Atlas* th_texture_atlas_load(const char* name) {
	GameState* gs = game_state();
	int x, y, comp;
	stbi_set_flip_vertically_on_load(1);
	uint8* data = stbi_load(name, &x, &y, &comp, 0);
	Assert(data);
	sg_range range = { data, x * y * 4 * sizeof(char) };
	sg_image_desc desc = { 0 };
	desc.width = x;
	desc.height = y;
	desc.data.subimage[0][0] = range;
	Atlas* atlas = TH_ARRAY_PUSH(gs->atlases, gs->atlas_count);
	atlas->image = sg_make_image(desc);
	strcpy(atlas->name, name);
	stbi_image_free(data);
	return atlas;
}

static sgp_rect range2_to_sgp_rect(range2 range) {
	sgp_rect result = { 0 };
	vec2 size = range2_size(range);
	result.x = range.min.x;
	result.y = range.min.y;
	result.w = size.x;
	result.h = size.y;
	return result;
}

static Sprite* th_texture_sprite_create(Atlas* atlas, const char* name, range2 sub_rect) {
	GameState* gs = game_state();
	Sprite* sprite = TH_ARRAY_PUSH(gs->sprites, gs->sprite_count);
	sprite->atlas = atlas;
	sprite->sub_rect = sub_rect;
	strcpy(sprite->name, name);
	return sprite;
}

static Sprite* th_texture_sprite_get(const char* name) {
	GameState* gs = game_state();
	for (int i = 0; i < gs->sprite_count; i++) {
		Sprite* sprite = &gs->sprites[i];
		if (strcmp(sprite->name, name) == 0) {
			return sprite;
		}
	}
	Assert(0);
	return 0;
}

static Entity* th_entity_create_plant() {
	WorldState* world = world_state();
	Entity* entity = TH_ARRAY_PUSH(world->entities, world->entity_count);
	entity->render = 1;
	entity->plant = 1;
	RenderRect* render = TH_ARRAY_PUSH(entity->render_rects, entity->render_rect_count);
	render->rect.max = vec2(16.0f, 64.0f);
	render->rect = range2_center_bottom(render->rect);
	render->col = TH_WHITE;
	return entity;
}

static void th_world_init(WorldState* world) {
	{
		// player
		Entity* entity = TH_ARRAY_PUSH(world->entities, world->entity_count);
		world->player = entity;
		entity->bounds.max = vec2(5.0f, 10.0f);
		entity->bounds = range2_center_bottom(entity->bounds);
		entity->pos.y = 100.0f;
		entity->rigid_body = 1;
		entity->render = 1;
		RenderRect* render = TH_ARRAY_PUSH(entity->render_rects, entity->render_rect_count);
		render->rect = entity->bounds;
		render->col = TH_WHITE;
		RenderRect* eye = TH_ARRAY_PUSH(entity->render_rects, entity->render_rect_count);
		eye->col = TH_BLACK;
		eye->rect.max = vec2(1, 1);
		eye->rect = range2_shift(eye->rect, vec2(1.0f, 8.0f));
	}
	{
		// test seed
		Entity* entity = TH_ARRAY_PUSH(world->entities, world->entity_count);
		entity->bounds.max = vec2(2.0f, 2.0f);
		entity->bounds = range2_center_bottom(entity->bounds);
		entity->render = 1;
		//entity->rigid_body = 1;
		RenderRect* render = TH_ARRAY_PUSH(entity->render_rects, entity->render_rect_count);
		render->rect = entity->bounds;
		render->col = TH_WHITE;
		world->held_seed = entity;
	}
	{
		Entity* entity = TH_ARRAY_PUSH(world->entities, world->entity_count);
		entity->bounds.max = vec2(4.0f, 4.0f);
		entity->bounds = range2_center_bottom(entity->bounds);
		entity->render = 1;
		RenderRect* render = TH_ARRAY_PUSH(entity->render_rects, entity->render_rect_count);
		render->rect = entity->bounds;
		render->col = TH_WHITE;
		render->sprite = th_texture_sprite_get("resource1");
	}
}

#endif