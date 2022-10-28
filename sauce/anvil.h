#ifndef ANVIL_H
#define ANVIL_H

#define APP_NAME "Game C"
#define MOVE_SPEED 2000.0f
#define PIXEL_SCALE 30.0f
#define GRAVITY 1000.0f
#define DEFAULT_CAMERA_SCALE 5.0f

global const S8 plant_stage_max = 6;
global const F32 plant_growth_speed = 0.2f;

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
	// flags
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

enum EntityType {
	ENTITY_null,
	ENTITY_seed,
	ENTITY_resource,
	ENTITY_plant,
	ENTITY_emitter,
};

typedef struct Entity Entity;
typedef struct EntityNode EntityNode;
struct EntityNode {
	EntityNode* next;
	Entity* entity;
};

struct Entity {
	U32 id;
	EntityFrame frame; // per-frame data, zeroed out at the start of each frame
	U32 children_entity_ids[8];
	EntityType type;
	B8 destroy_at_frame_end; // @arena?
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
	S8 plant_stage;
	F32 zero_timer;
	B8 interactable;
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
	Camera cam;

	TextureAtlas atlases[16];
	U32 atlas_count;
	Sprite sprites[64];
	U32 sprite_count;

	Emitter emitters[16];
	U32 emitter_count;
	Particle particles[2048];
	U32 particle_count;

	EntityNode* first_entity;

	// per-frame
	Vec2 mouse_pos;
	Vec2 window_size;
	B8 key_pressed[SAPP_KEYCODE_MENU];
	B8 key_released[SAPP_KEYCODE_MENU];
	B8 mouse_pressed[SAPP_MOUSEBUTTON_MIDDLE];
	B8 mouse_released[SAPP_MOUSEBUTTON_MIDDLE];
};

static GameState* game_state() {
	static GameState gs = { 0 };
	return &gs;
}
static WorldState* world_state() {
	GameState* gs = game_state();
	return &gs->world_state;
}

typedef struct GameMemory GameMemory;
struct GameMemory
{
	M_Arena permanent_arena;
	M_Arena world_arena;
	M_Arena frame_arena;
};
global GameMemory game_memory = { 0 };

function M_Arena* TH_PermanentArena()
{
	return &game_memory.permanent_arena;
}
function M_Arena* TH_WorldArena()
{
	return &game_memory.world_arena;
}
function M_Arena* TH_FrameArena()
{
	return &game_memory.frame_arena;
}

#ifdef FUN_VAL
static F32 fun_val = 0.0f;
#endif

// to move this system to pooling, or not to? That is the question.
// 

function Entity* EntityCreate()
{
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

function B8 EntityHasChildren(Entity* entity) {
	ForEach(id, entity->children_entity_ids, U32*) {
		if (*id != 0) {
			return 1;
		}
	}
	return 0;
}

function void EntityPushChild(Entity* entity, U32 child_id) {
	U32* free_slot = 0;
	ForEach(id, entity->children_entity_ids, U32*) {
		if (*id == 0) {
			free_slot = id;
			break;
		}
	}
	Assert(free_slot); // entity has reached max children
	*free_slot = child_id;
}

function B8 EntityDetachFromParent(Entity* child)
{
	Assert(child && child->id);
	WorldState* world = world_state();
	// loop over all entities and clear self from children
	B8 found = 0;
	ForEach(entity, world->entities, Entity*)
	{
		ForEach(id, entity->children_entity_ids, U32*)
		{
			if (*id == child->id)
			{
				*id = 0;
				found = 1;
			}
		}
	}
	return found;
}

static Rng2F32 camera_get_bounds() {
	GameState* gs = game_state();
	// todo - @camera
	Rng2F32 cam = { 0 };
	cam.max = Vec2((int)gs->window_size.x, (int)gs->window_size.y);
	cam = Shift2F32(cam, Vec2((int)gs->window_size.x, (int)gs->window_size.y) * -0.5f);
	return cam;
}

function void emitter_ambient_screen(Particle* particle, Emitter* emitter)
{
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

function void EmitterPlantSheer(Particle* particle, Emitter* emitter)
{
	particle->pos = emitter->pos;
	particle->vel = Vec2(float_random_range(-20.f, 20.f), float_random_range(-20.f, 20.f));
	particle->col = Vec4(0.7f, 0.7f, 0.7f, 1.0f);
	particle->start_life = particle->life = 0.2f;
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

static Entity* EntityCreatePlant() {
	WorldState* world = world_state();
	Entity* entity = EntityCreate();
	entity->sprite = th_texture_sprite_get("plant0"); // first default
	th_entity_set_bounds_from_sprite(entity);
	entity->render = 1;
	entity->col = TH_WHITE;
	entity->type = ENTITY_plant;
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
	entity->type = ENTITY_resource;
	return entity;
}

function Entity* EntityCreateSeed()
{
	Entity* entity = EntityCreate();
	entity->bounds.max = Vec2(2.0f, 2.0f);
	entity->bounds = range2_center_bottom(entity->bounds);
	entity->render = 1;
	entity->interactable = 1;
	entity->rigid_body = 1;
	entity->render_rect = entity->bounds;
	entity->col = TH_WHITE;
	entity->type = ENTITY_seed;
	return entity;
}

function Emitter* CreateEmitter()
{
	GameState* gs = game_state();
	Emitter* emit = TH_ARRAY_PUSH(gs->emitters, gs->emitter_count);
	return emit;
}

static void th_world_init(WorldState* world) {
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

	// is all begins with...
	entity = EntityCreateSeed();
	
	// test resource
	entity = EntityCreateResource();
	entity->pos.x = 100.0f;
	
	// test plant
	entity = EntityCreatePlant();
	entity->pos.x = 20;
	entity->plant_stage = plant_stage_max;
	entity = EntityCreatePlant();
	entity->pos.x = 50;
	entity->plant_stage = plant_stage_max;
	entity = EntityCreatePlant();
	entity->pos.x = -50.0f;
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

function void ProcessPlayerInteraction(TH_Coroutine* coro, FrameState* frame) {
	GameState* gs = game_state();
	WorldState* world = world_state();
	Entity* held_entity = EntityFromID(world->held_entity_id);
	const Vec2 world_mouse = mouse_pos_in_worldspace();

	TH_CoroutineBegin(coro);

	while (!(gs->key_pressed[SAPP_KEYCODE_E] && frame->hovered_entity))
	{
		if (frame->hovered_entity)
			frame->hovered_entity->frame.render_highlight = 1;

		TH_CoroutineYield(coro);
	}
	gs->key_pressed[SAPP_KEYCODE_E] = 0; // todo - auto sponge the press?

	if (frame->hovered_entity->type == ENTITY_seed)
	{
		world->held_entity_id = frame->hovered_entity->id;
		held_entity = EntityFromID(world->held_entity_id);

		while (!gs->mouse_pressed[SAPP_MOUSEBUTTON_LEFT])
		{
			Assert(held_entity);
			held_entity->pos.x = roundf(world_mouse.x);
			held_entity->pos.y = 0.0f;
			sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
			sgp_draw_line(held_entity->pos.x, world_mouse.y, held_entity->pos.x, 0.0f);

			TH_CoroutineYield(coro);
		}
		gs->mouse_pressed[SAPP_MOUSEBUTTON_LEFT] = 0;

		Assert(held_entity);
		Entity* plant = EntityCreatePlant();
		plant->pos = held_entity->pos;
		EntityDestroy(held_entity);
	}
	else if (frame->hovered_entity->type == ENTITY_plant)
	{
		// snip snip
		Vec2 spawn_pos = frame->hovered_entity->pos;
		EntityDestroy(frame->hovered_entity);
		Entity* seed = EntityCreateSeed();
		spawn_pos.y += 20.0f;
		seed->pos = spawn_pos;

		// fire off emitter
		Emitter* emit = CreateEmitter();
		emit->pos = spawn_pos;
		emit->emit_func = EmitterPlantSheer;
		emit->frequency = 20.0f;
		// todo - frame arena

		// wait

		// todo @polish @thread ? I almost just want to fire off another thread of execution here so I can do the timer elsewhere instead of stopping this coro
		// duplication animation. wait 1 second, but animate it shaking like a poke ball, then poppin out once done
		// it can't be stopping this coroutine though...
	}
	else if (frame->hovered_entity->type == ENTITY_resource)
	{
		world->held_entity_id = frame->hovered_entity->id;
		held_entity = EntityFromID(world->held_entity_id);
		EntityDetachFromParent(frame->hovered_entity);
		// todo - parent id

		while (!gs->key_pressed[SAPP_KEYCODE_E])
		{
			Assert(held_entity);
			// @ship note - can I implicitly assert this just by accessing it? How can I get neat crashes when just straight up accessing a null pointer?
			held_entity->vel = frame->player->vel;
			held_entity->pos = frame->player->pos;
			held_entity->pos.x += 7.0f * frame->player->x_dir;
			held_entity->pos.y += 10.0f;

			TH_CoroutineYield(coro);
		}
		gs->key_pressed[SAPP_KEYCODE_E] = 0;

		Assert(held_entity);
		gs->world_state.held_entity_id = 0;
		held_entity->vel.x += frame->player->x_dir * 100.0f;
		held_entity->rigid_body = 1;
	}

	TH_CoroutineReset(coro);
}

#endif