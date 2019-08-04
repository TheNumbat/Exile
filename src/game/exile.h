
#pragma once

#include <engine/engine.h>
#include <engine/events.h>

#include "world.h"
#include "gfx.h"

struct exile_state {

	engine* eng = null;
	
	evt_state_machine controls;
	evt_state_id camera_evt = 0, ui_evt = 0;
	evt_handler_id default_evt = 0;

	world w;
	exile_renderer ren;
	
	platform_allocator a_assets, a_world, a_render, a_general;

	asset_store store;

	void init();
	void destroy();
	void gl_reload();

	void update();
	void render();
};

// TODO(max): not great to have this global; don't need this _and_ the engine to be separately allocated,
// causing a lot of double indirections. This is all really just a product of hot reloading, which should
// probably be scrapped tbh.
extern exile_state* exile;

CALLBACK void camera_to_ui(void* param);
CALLBACK void ui_to_camera(void* param);

CALLBACK bool default_evt_handle(void* param, platform_event evt);
CALLBACK bool camera_evt_handle(void* param, platform_event evt);
CALLBACK bool ui_evt_handle(void* param, platform_event evt);

void* start_up_game(engine* e);

void gl_reload_game(void* game);
void run_game(void* game);
void shut_down_game(void* game);
void unload_game(engine* e, void* game);
void reload_game(engine* e, void* game);
