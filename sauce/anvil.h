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

#define TH_BLACK V4(0.0f, 0.0f, 0.0f, 1.0f)
#define TH_WHITE V4(1.0f, 1.0f, 1.0f, 1.0f)

typedef struct Entity Entity;
typedef struct Particle Particle;
typedef void (*ParticleEmitterFunc)(Particle*, Entity*);

struct Particle
{
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

typedef struct TextureAtlas TextureAtlas;
struct TextureAtlas
{
	char name[128];
	sg_image image;
};

typedef struct Sprite Sprite;
struct Sprite
{
	char name[128];
	TextureAtlas* atlas;
	Rng2F32 sub_rect;
};

typedef struct RenderRect RenderRect;
struct RenderRect
{
	Rng2F32 rect;
	Sprite* sprite;
	Vec4 col;
};

typedef struct EntityFrame EntityFrame;
struct EntityFrame
{
	// per-frame data, zeroed out at the start of each frame
	B8 render_highlight;
};

typedef enum EntityType EntityType;
enum EntityType
{
	ENTITY_null,
	ENTITY_particle_emitter,
	ENTITY_seed,
	ENTITY_resource,
	ENTITY_plant,
};

struct Entity
{
	U64 id;
	EntityFrame frame;
	U32 children_entity_ids[8];
	EntityType type;
	U32 lifetime_ticks_remaining;

	// PHYSICS
	B8 rigid_body;
	Vec2 pos;
	Vec2 vel;
	Vec2 acc;
	Rng2F32 bounds;
	B8 x_friction_mult;

	// RENDER
	B8 render;
	Rng2F32 render_rect;
	Sprite* sprite;
	Vec4 col;
	B8 x_dir;
	Vec2 render_offset;

	// PARTICLE
	ParticleEmitterFunc emit_func;
	F32 frequency;

	// UPDATE LOGIC
	Coroutine update_coro;
	B8 interactable;
	S8 plant_stage;
	S8 resource_stage;
};

typedef struct Camera Camera;
struct Camera
{
	Vec2 pos;
	F32 scale;
	F32 rotation;
};

typedef struct FrameState FrameState;
struct FrameState
{
	Entity* player;
	Entity* hovered_entity;
};

typedef struct WorldState WorldState;
struct WorldState
{
	Entity entities[64];
	U32 last_entity_id;
	Entity* player_temp; // todo - change to ID
	U32 held_entity_id;
};

typedef struct GameState GameState;
struct GameState
{
	WorldState world_state;
	B8 key_down[SAPP_KEYCODE_MENU];
	B8 mouse_down[SAPP_MOUSEBUTTON_MIDDLE];
	Camera cam;

	TextureAtlas atlases[16];
	U32 atlas_count;
	Sprite sprites[64];
	U32 sprite_count;

	Particle particles[2048];
	U32 particle_count;

	// per-frame
	F32 delta_t;
	Vec2 mouse_pos;
	Vec2 window_size;
	B8 key_pressed[SAPP_KEYCODE_MENU];
	B8 key_released[SAPP_KEYCODE_MENU];
	B8 mouse_pressed[SAPP_MOUSEBUTTON_MIDDLE];
	B8 mouse_released[SAPP_MOUSEBUTTON_MIDDLE];
};

function GameState* game_state() {
	function GameState gs = { 0 };
	return &gs;
}
function WorldState* world_state() {
	GameState* gs = game_state();
	return &gs->world_state;
}

function F32 APP_DT()
{
	GameState* gs = game_state();
	return gs->delta_t;
}

typedef struct GameMemory GameMemory;
struct GameMemory
{
	M_Arena permanent_arena;
	M_Arena world_arena;
	M_Arena frame_arena;
	M_Arena last_frame_arena;
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
function F32 fun_val = 0.0f;
#endif

function Entity* EntityCreate()
{
	WorldState* world = world_state();
	Assert(world->last_entity_id + 1 < U64Max); // I don't think we'll ever hit this number of ids lmfao
	world->last_entity_id++;
	ForEachFlat(entity, world->entities, Entity*)
	{
		if (!entity->id)
		{
			entity->id = world->last_entity_id;
			return entity;
		}
	}
	Assert(0 && "no more free entities :(");
	return 0;
}

function void EntityDestroy(Entity* entity) {
	MemoryZeroStruct(entity);
}

function Entity* EntityFromID(U32 id) {
	if (id == 0)
		return 0;
	WorldState* world = world_state();
	ForEachFlat(entity, world->entities, Entity*) {
		if (entity->id == id) {
			return entity;
		}
	}
	return 0;
}

function B8 EntityHasChildren(Entity* entity) {
	ForEachFlat(id, entity->children_entity_ids, U32*) {
		if (*id != 0) {
			return 1;
		}
	}
	return 0;
}

function void EntityPushChild(Entity* entity, U32 child_id) {
	U32* free_slot = 0;
	ForEachFlat(id, entity->children_entity_ids, U32*) {
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
	ForEachFlat(entity, world->entities, Entity*)
	{
		ForEachFlat(id, entity->children_entity_ids, U32*)
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

function Rng2F32 camera_get_bounds()
{
	GameState* gs = game_state();
	// todo - @camera
	Rng2F32 cam = { 0 };
	cam.max = V2(gs->window_size.x, gs->window_size.y);
	cam = Shift2F32(cam, Scale2F32(V2(gs->window_size.x, gs->window_size.y), -0.5f));
	return cam;
}

// ideally this is just the inverse of the projection matrix, but I don't have the paitence
// @camera - construct my own 2x3 camera view matrix and pass it down to sokol_gp somehow by pushing it as a transform?
// or maybe just push n pop properly? idk
function Vec2 world_pos_to_screen_pos(Vec2 world_pos, Camera cam)
{
	GameState* gs = game_state();
	Vec2 result = world_pos;
	result = Sub2F32(result, cam.pos);
	result.x *= cam.scale;
	result.y *= cam.scale;
	result = Add2F32(Scale2F32(gs->window_size, 0.5f), result);
	result.y = gs->window_size.y - result.y;
	return result;
}

function Vec2 screen_pos_to_world_pos(Vec2 screen_pos, Camera cam) {
	GameState* gs = game_state();
	Vec2 result = screen_pos;
	result.y = gs->window_size.y - result.y;
	result = Add2F32(Scale2F32(gs->window_size, -0.5f), result);
	result.x /= cam.scale;
	result.y /= cam.scale;
	result = Add2F32(cam.pos, result);
	return result;
}

function Vec2 mouse_pos_in_worldspace() {
	GameState* gs = game_state();
	return screen_pos_to_world_pos(gs->mouse_pos, gs->cam);
}

function U32 c_string_length(const char* string) {
	const char* cursor = string;
	while (cursor[0] != '\0') {
		cursor++;
	}
	U32 length = cursor - string;
	return length;
}

function U32 hash_from_string(const char* string) {
	U32 result = 5381;
	U32 string_size = c_string_length(string);
	for (int i = 0; i < string_size; i++) {
		result = ((result << 5) + result) + string[i];
	}
	return result;
}

function TextureAtlas* th_texture_atlas_get(const char* string) {
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

function TextureAtlas* th_texture_atlas_load(const char* name) {
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
	atlas->image = sg_make_image(&desc);
	strcpy(atlas->name, name);
	stbi_image_free(data);
	return atlas;
}

function sgp_rect range2_to_sgp_rect(Rng2F32 range) {
	sgp_rect result = { 0 };
	Vec2 size = Dim2F32(range);
	result.x = range.min.x;
	result.y = range.min.y;
	result.w = size.x;
	result.h = size.y;
	return result;
}

function Sprite* TextureSpriteCreate(TextureAtlas* atlas, const char* name, Rng2F32 sub_rect) {
	GameState* gs = game_state();
	Sprite* sprite = TH_ARRAY_PUSH(gs->sprites, gs->sprite_count);
	sprite->atlas = atlas;
	sprite->sub_rect = sub_rect;
	strcpy(sprite->name, name);
	return sprite;
}

function Sprite* th_texture_sprite_get(const char* name) {
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

function void EntitySetBoundsFromSprite(Entity* entity)
{
	Assert(entity->sprite);
	entity->bounds = range2_remove_offset(entity->sprite->sub_rect);
	entity->bounds = range2_center_bottom(entity->bounds);
	entity->render_rect = entity->bounds;
}

function Entity* EntityCreateEmitter()
{
	Entity* entity = EntityCreate();
	entity->type = ENTITY_particle_emitter;
	return entity;
}

function Rng2F32 EntityBoundsInWorld(const Entity* entity)
{
	Rng2F32 result = entity->bounds;
	result = Shift2F32(result, entity->pos);
	return result;
}

function void sgp_draw_debug_rect_lines(Rng2F32 rect)
{
	sgp_draw_line(rect.min.x, rect.min.y, rect.min.x, rect.max.y);
	sgp_draw_line(rect.min.x, rect.min.y, rect.max.x, rect.min.y);
	sgp_draw_line(rect.max.x, rect.max.y, rect.min.x, rect.max.y);
	sgp_draw_line(rect.max.x, rect.max.y, rect.max.x, rect.min.y);
}

#endif