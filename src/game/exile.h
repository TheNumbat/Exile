
#pragma once

struct exile_state {

	engine* eng = null;
	
	evt_state_machine controls;
	evt_state_id camera_evt = 0, ui_evt = 0;
	evt_handler_id default_evt = 0;

	world w;
	exile_renderer ren;
	platform_allocator alloc;

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
static exile_state* exile = null;

CALLBACK void camera_to_ui(void* param);
CALLBACK void ui_to_camera(void* param);

CALLBACK bool default_evt_handle(void* param, platform_event evt);
CALLBACK bool camera_evt_handle(void* param, platform_event evt);
CALLBACK bool ui_evt_handle(void* param, platform_event evt);

void* start_up_game(engine* e) {
	exile = new (e->platform->heap_alloc(sizeof(exile_state))) exile_state;
	exile->eng = e;
	exile->init();
	return exile;
}

void gl_reload_game(void* game) {
	exile->gl_reload();
}

void run_game(void* game) {
	exile->update();
	exile->render();
}

void shut_down_game(void* game) {
	exile->destroy();
	exile->eng->platform->heap_free(exile);
}

void unload_game(engine* e, void* game) {
	exile->w.thread_pool.stop_all();
}

void reload_game(engine* e, void* game) {
	exile = (exile_state*)game;
	exile->w.thread_pool.start_all();
}
