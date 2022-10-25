#ifndef ANVIL_H
#define ANVIL_H

#define APP_NAME "Game C"
#define MOVE_SPEED 2000.0f
#define PIXEL_SCALE 30.0f
#define GRAVITY 1000.0f
#define DEFAULT_CAMERA_SCALE 5.0f

#ifndef TH_SHIP
//#define FUN_VAL
//#define RENDER_COLLIDERS
#define RENDER_COLLIDER_COLOR 1.0f, 0.5f, 0.5f, 1.0f
#endif

// helpers
#define TH_BLACK Vec4(0.0f, 0.0f, 0.0f, 1.0f)
#define TH_WHITE Vec4(1.0f, 1.0f, 1.0f, 1.0f)

typedef struct Emitter Emitter;
typedef struct Particle Particle;
typedef void (*ParticleEmitterFunc)(Particle*, Emitter*);
#define PARTICLE_EMITTER_FUNC(function_name) static void function_name(Particle* particle, Emitter* emitter)

struct Emitter {
	Vec2 pos;
	F32 frequency;
	ParticleEmitterFunc emit_func;
};

struct Particle {
	Vec2 pos;
	Vec2 vel;
	Vec4 col;
	F32 start_life;
	F32 life;
	F32 size_mult;
	// flags;
	B8 fade_in;
	B8 fade_out;
};

struct TextureAtlas {
	char name[128];
	sg_image image;
};

struct Sprite {
	char name[128];
	TextureAtlas* atlas;
	Rng2F32 sub_rect;
};

struct RenderRect {
	Rng2F32 rect;
	Sprite* sprite;
	Vec4 col;
};

struct EntityFrame {
	B8 render_highlight;
};

struct Entity {
	U32 id;
	EntityFrame frame; // per-frame data, zeroed out at the start of each frame
	B8 rigid_body;
	Vec2 pos;
	Vec2 vel;
	Vec2 acc;
	Rng2F32 bounds;
	B8 x_friction_mult;
	B8 render;
	Rng2F32 render_rect;
	Sprite* sprite;
	Vec4 col;
	B8 x_dir;
	B8 plant;
	F32 plant_stage;
	B8 interactable;
	B8 seed;
	B8 resource;
};

struct Camera {
	Vec2 pos;
	F32 scale;
	F32 rotation;
};

struct FrameState {
	Entity* player;
	Entity* hovered_entity;
};

struct WorldState {
	Entity entities[64];
	U32 last_entity_id;
	Entity* player_temp; // todo - change to ID
	U32 held_entity_id;
};

struct GameState {
	WorldState world_state;
	B8 key_down[SAPP_KEYCODE_MENU];
	B8 mouse_down[SAPP_MOUSEBUTTON_MIDDLE];
	Emitter emitters[16];
	U32 emitter_count;
	Particle particles[2048];
	U32 particle_count;
	Camera cam;
	TextureAtlas atlases[16];
	U32 atlas_count;
	Sprite sprites[64];
	U32 sprite_count;
	// per-frame
	Vec2 mouse_pos;
	Vec2 window_size;
	B8 key_pressed[SAPP_KEYCODE_MENU];
	B8 key_released[SAPP_KEYCODE_MENU];
	B8 mouse_pressed[SAPP_MOUSEBUTTON_MIDDLE];
	B8 mouse_released[SAPP_MOUSEBUTTON_MIDDLE];
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
static F32 fun_val = 0.0f;
#endif

function Entity* EntityCreate() {
	WorldState* world = world_state();
	Assert(world->last_entity_id + 1 < U32Max); // todo - more sophisticated IDs
	world->last_entity_id++;
	ForEach(entity, world->entities, Entity*) {
		if (!entity->id) {
			entity->id = world->last_entity_id;
			return entity;
		}
	}
	Assert(0); // no more free entities :(
	return 0;
}

function void EntityDestroy(Entity* entity) {
	MemoryZeroStruct(entity);
}

function Entity* EntityFromID(U32 id) {
	if (id == 0)
		return 0;
	WorldState* world = world_state();
	ForEach(entity, world->entities, Entity*) {
		if (entity->id == id) {
			return entity;
		}
	}
	return 0;
}

static Rng2F32 camera_get_bounds() {
	GameState* gs = game_state();
	// todo - @camera
	Rng2F32 cam = { 0 };
	cam.max = Vec2((int)gs->window_size.x, (int)gs->window_size.y);
	cam = Shift2F32(cam, Vec2((int)gs->window_size.x, (int)gs->window_size.y) * -0.5f);
	return cam;
}

PARTICLE_EMITTER_FUNC(emitter_ambient_screen) {
	Rng2F32 bounds = camera_get_bounds();
	particle->pos.x = float_random_range(bounds.min.x, bounds.max.x);
	particle->pos.y = float_random_range(bounds.min.y, bounds.max.y);
	particle->vel = Vec2(float_random_range(-1.f, 1.f), float_random_range(2.f, 4.f));
	particle->col = Vec4(0.7f, 0.7f, 0.7f, 1.0f);
	particle->start_life = particle->life = 2.f;
	particle->fade_in = 1;
	particle->fade_out = 1;
	particle->size_mult = 1.f;
}

// ideally this is just the inverse of the projection matrix, but I don't have the paitence
// @camera - construct my own 2x3 camera view matrix and pass it down to sokol_gp somehow by pushing it as a transform?
// or maybe just push n pop properly? idk
static Vec2 world_pos_to_screen_pos(const Vec2& world_pos, const Camera& cam) {
	GameState* gs = game_state();
	Vec2 result = world_pos;
	result = result - cam.pos;
	result.x *= cam.scale;
	result.y *= cam.scale;
	result += gs->window_size * 0.5f;
	result.y = gs->window_size.y - result.y;
	return result;
}

static Vec2 screen_pos_to_world_pos(const Vec2& screen_pos, const Camera& cam) {
	GameState* gs = game_state();
	Vec2 result = screen_pos;
	result.y = gs->window_size.y - result.y;
	result += gs->window_size * -0.5f;
	result.x /= cam.scale;
	result.y /= cam.scale;
	result += cam.pos;
	return result;
}

static Vec2 mouse_pos_in_worldspace() {
	GameState* gs = game_state();
	return screen_pos_to_world_pos(gs->mouse_pos, gs->cam);
}

static U32 c_string_length(const char* string) {
	const char* cursor = string;
	while (cursor[0] != '\0') {
		cursor++;
	}
	U32 length = cursor - string;
	return length;
}

static U32 hash_from_string(const char* string) {
	U32 result = 5381;
	U32 string_size = c_string_length(string);
	for (int i = 0; i < string_size; i++) {
		result = ((result << 5) + result) + string[i];
	}
	return result;
}

static TextureAtlas* th_texture_atlas_get(const char* string) {
	GameState* gs = game_state();
	for (int i = 0; i < gs->atlas_count; i++) {
		TextureAtlas* texture = &gs->atlases[i];
		if (strcmp(texture->name, string) == 0) {
			return texture;
		}
	}
	Assert(0); // no texture found :(
	return 0;
}

static TextureAtlas* th_texture_atlas_load(const char* name) {
	GameState* gs = game_state();
	int x, y, comp;
	stbi_set_flip_vertically_on_load(1);
	U8* data = stbi_load(name, &x, &y, &comp, 0);
	Assert(data);
	sg_range range = { data, x * y * 4 * sizeof(U8) };
	sg_image_desc desc = { 0 };
	desc.width = x;
	desc.height = y;
	desc.data.subimage[0][0] = range;
	TextureAtlas* atlas = TH_ARRAY_PUSH(gs->atlases, gs->atlas_count);
	atlas->image = sg_make_image(desc);
	strcpy(atlas->name, name);
	stbi_image_free(data);
	return atlas;
}

static sgp_rect range2_to_sgp_rect(Rng2F32 range) {
	sgp_rect result = { 0 };
	Vec2 size = Dim2F32(range);
	result.x = range.min.x;
	result.y = range.min.y;
	result.w = size.x;
	result.h = size.y;
	return result;
}

static Sprite* th_texture_sprite_create(TextureAtlas* atlas, const char* name, Rng2F32 sub_rect) {
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

// ENTITY HELPERS

static void th_entity_set_bounds_from_sprite(Entity* entity) {
	Assert(entity->sprite);
	entity->bounds = range2_remove_offset(entity->sprite->sub_rect);
	entity->bounds = range2_center_bottom(entity->bounds);
	entity->render_rect = entity->bounds;
}

static Entity* th_entity_create_plant() {
	WorldState* world = world_state();
	Entity* entity = EntityCreate();
	entity->sprite = th_texture_sprite_get("plant0"); // first default
	th_entity_set_bounds_from_sprite(entity);
	entity->render = 1;
	entity->plant = 1;
	entity->col = TH_WHITE;
	entity->interactable = 1;
	return entity;
}

function Entity* EntityCreateResource() {
	WorldState* world = world_state();
	Entity* entity = EntityCreate();
	entity->sprite = th_texture_sprite_get("resource1");
	th_entity_set_bounds_from_sprite(entity);
	entity->render = 1;
	entity->interactable = 1;
	entity->rigid_body = 1;
	entity->x_friction_mult = 4.0f;
	entity->col = TH_WHITE;
	entity->resource = 1;
	return entity;
}

static void th_world_init(WorldState* world) {
	{
		// player
		Entity* entity = EntityCreate();
		world->player_temp = entity;
		entity->sprite = th_texture_sprite_get("arcane_player");
		th_entity_set_bounds_from_sprite(entity);
		entity->pos.y = 100.0f;
		entity->rigid_body = 1;
		entity->render = 1;
		entity->x_friction_mult = 15.0f;
		entity->col = TH_WHITE;
	}
	{
		// starter seed
		Entity* entity = EntityCreate();
		entity->bounds.max = Vec2(2.0f, 2.0f);
		entity->bounds = range2_center_bottom(entity->bounds);
		entity->render = 1;
		entity->interactable = 1;
		entity->seed = 1;
		//entity->rigid_body = 1;
		entity->render_rect = entity->bounds;
		entity->col = TH_WHITE;
		// world->held_seed = entity;
	}
	{
		// test resource
		Entity* entity = EntityCreateResource();
		entity->pos.x = 100.0f;
	}
	{
		// test plant
		Entity* entity = th_entity_create_plant();
		entity->pos.x = -50.0f;
	}
}

static void sgp_draw_debug_rect_lines(Rng2F32 rect) {
	sgp_draw_line(rect.min.x, rect.min.y, rect.min.x, rect.max.y);
	sgp_draw_line(rect.min.x, rect.min.y, rect.max.x, rect.min.y);
	sgp_draw_line(rect.max.x, rect.max.y, rect.min.x, rect.max.y);
	sgp_draw_line(rect.max.x, rect.max.y, rect.max.x, rect.min.y);
}

static Rng2F32 EntityBoundsInWorld(const Entity* entity) {
	Rng2F32 result = entity->bounds;
	result = Shift2F32(result, entity->pos);
	return result;
}

function void ProcessPlayerInteraction(TH_Coroutine* coro, FrameState* st) {
	GameState* gs = game_state();

	TH_CoroutineBegin(coro);

	TH_CoroutineYieldUntil(coro, st->hovered_entity &&
												 gs->key_pressed[SAPP_KEYCODE_E] &&
												 st->hovered_entity->interactable);

	// pick up hovered interactable
	gs->world_state.held_entity_id = st->hovered_entity->id;
	gs->key_pressed[SAPP_KEYCODE_E] = 0; // sponge keypress

	TH_CoroutineYieldUntil(coro, gs->key_pressed[SAPP_KEYCODE_E]);

	{
		Entity* held_entity = EntityFromID(gs->world_state.held_entity_id);
		Assert(held_entity);
		if (held_entity->seed)
		{
			// place plant
			Entity* plant = th_entity_create_plant();
			plant->pos = held_entity->pos;
			EntityDestroy(held_entity);
		}
		else
		{
			// throw it with velocity
			gs->world_state.held_entity_id = 0;
			held_entity->vel.x += st->player->x_dir * 100.0f;
			held_entity->rigid_body = 1;
		}
	}

	TH_CoroutineReset(coro);
}

#endif