// Game C specific stuff.

global const S8 plant_stage_max = 6;
global const F32 plant_growth_speed = 0.2f;

function Entity* EntityCreatePlant()
{
	WorldState* world = world_state();
	Entity* entity = EntityCreate();
	entity->sprite = th_texture_sprite_get("plant0"); // first default
	EntitySetBoundsFromSprite(entity);
	entity->render = 1;
	entity->col = TH_WHITE;
	entity->type = ENTITY_plant;
	return entity;
}

function Entity* EntityCreateResource()
{
	WorldState* world = world_state();
	Entity* entity = EntityCreate();
	entity->sprite = th_texture_sprite_get("resource1");
	EntitySetBoundsFromSprite(entity);
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
	entity->x_friction_mult = 5.0f;
	entity->col = TH_WHITE;
	entity->type = ENTITY_seed;
	return entity;
}

function void emitter_ambient_screen(Particle* particle, Entity* emitter)
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

function void EmitterPlantSheer(Particle* particle, Entity* emitter)
{
	particle->pos = emitter->pos;
	particle->vel = Vec2(float_random_range(-20.f, 20.f), float_random_range(-20.f, 20.f));
	particle->col = Vec4(0.7f, 0.7f, 0.7f, 1.0f);
	particle->start_life = particle->life = 0.2f;
	particle->fade_in = 1;
	particle->fade_out = 1;
	particle->size_mult = 1.f;
}

function void CoroSeedUpdate(Entity* seed)
{
	GameState* gs = game_state();
	WorldState* world = world_state();
	Coroutine* coro = &seed->update_coro;

	CoroutineBegin(coro);

	// pokeball :)
	CoroutineWait(coro, .5f);

	{
		Entity* new_seed = EntityCreateSeed();
		new_seed->pos = seed->pos;
		new_seed->vel.x = 100.0f;
		new_seed->vel.y = 100.0f;
	}
	
	CoroutineExit(coro);
}

function void ProcessPlayerInteraction(Coroutine* coro, FrameState* frame)
{
	GameState* gs = game_state();
	WorldState* world = world_state();
	Entity* held_entity = EntityFromID(world->held_entity_id);
	const Vec2 world_mouse = mouse_pos_in_worldspace();

	CoroutineBegin(coro);

	while (!(gs->key_pressed[SAPP_KEYCODE_E] && frame->hovered_entity))
	{
		if (frame->hovered_entity)
			frame->hovered_entity->frame.render_highlight = 1;

		CoroutineYield(coro);
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

			CoroutineYield(coro);
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
		Entity* emit = EntityCreateEmitter();
		emit->pos = spawn_pos;
		emit->emit_func = EmitterPlantSheer;
		emit->frequency = 20.0f;
		emit->lifetime_ticks_remaining = 1;
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

			CoroutineYield(coro);
		}
		gs->key_pressed[SAPP_KEYCODE_E] = 0;

		Assert(held_entity);
		gs->world_state.held_entity_id = 0;
		held_entity->vel.x += frame->player->x_dir * 100.0f;
		held_entity->rigid_body = 1;
	}

	CoroutineReset(coro);
}

function void WorldInit(WorldState* world)
{
	// player
	Entity* entity = EntityCreate();
	world->player_temp = entity;
	entity->sprite = th_texture_sprite_get("arcane_player");
	EntitySetBoundsFromSprite(entity);
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