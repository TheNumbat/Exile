
#pragma once

struct game_state {
	platform_api* api = NULL;
	
	arena_allocator 	transient_arena;
	platform_allocator 	default_platform_allocator, suppressed_platform_allocator;

	bool running = false;

	log_manager log;
	ogl_manager ogl;
	gui_manager gui;
	dbg_manager dbg;
	evt_manager evt;
	threadpool  thread_pool;
	platform_allocator log_a, ogl_a, gui_a, dbg_a, evt_a, thread_pool_a; // idk about this

	platform_window window 	= {};
	i32 window_w = 0, window_h = 0;

	// testing
	asset_store default_store;
	platform_allocator default_store_a;
};
