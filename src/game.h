
#pragma once

struct game_state {
	platform_api* api = NULL;
	
	arena_allocator 	transient_arena;
	platform_allocator 	default_platform_allocator;
	platform_allocator 	suppressed_platform_allocator;

	bool running = false;

	log_manager log;
	ogl_manager ogl;
	gui_manager gui;
	dbg_manager dbg;
	evt_manager evt;
	threadpool thread_pool;

	platform_window window 	= {};
	i32 window_w = 0, window_h = 0;

	// testing
	asset_store default_store;
};
