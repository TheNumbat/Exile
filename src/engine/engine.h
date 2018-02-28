
#pragma once

struct engine {

// private
	arena_allocator 	transient_arena;
	platform_allocator 	default_platform_allocator, suppressed_platform_allocator;

	func_ptr_state func_state;
	asset_store default_store;

	platform_allocator log_a, ogl_a, gui_a, dbg_a, evt_a, thread_pool_a; // idk about this
	
	void* game_state = null;
//

// API
	log_manager log;
	ogl_manager ogl;
	gui_manager gui;
	dbg_manager dbg;
	evt_manager evt;
	threadpool  thread_pool;

	bool running = false;
	platform_window window;
	platform_api* platform = null;
//
};

void* start_up_game(engine* e);
void run_game(void* game);
void shut_down_game(void* game);
void reload_game(engine* e, void* game);
