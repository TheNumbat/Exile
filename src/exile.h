
#pragma once

struct exile {

	evt_state_machine controls;
	evt_state_id camera_evt = 0, ui_evt = 0;
	evt_handler_id default_evt = 0;

	world w;
	platform_allocator alloc;

	asset_store store;

	void init();
	void destroy();
	void gl_reload();

	void update();
	void render();
};

CALLBACK void camera_to_ui(void* param);
CALLBACK void ui_to_camera(void* param);

CALLBACK bool default_evt_handle(void* param, platform_event evt);
CALLBACK bool camera_evt_handle(void* param, platform_event evt);
CALLBACK bool ui_evt_handle(void* param, platform_event evt);

void* start_up_game(engine* e) {
	eng = e;
	exile* ret = new(eng->platform->heap_alloc(sizeof(exile)))exile;
	ret->init();
	return ret;
}

void gl_reload_game(void* game) {
	exile* ex = (exile*)game;
	ex->gl_reload();
}

void run_game(void* game) {
	exile* e = (exile*)game;
	e->update();
	e->render();
}

void shut_down_game(void* game) {
	exile* e = (exile*)game;
	e->destroy();
	eng->platform->heap_free(e);
}

void unload_game(engine* e, void* game) {
	exile* ex = (exile*)game;
	ex->w.thread_pool.stop_all();
}

void reload_game(engine* e, void* game) {
	eng = e;
	exile* ex = (exile*)game;
	ex->w.thread_pool.start_all();
}
