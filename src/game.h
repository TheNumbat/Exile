
#pragma once

struct test {
	vector<i32> v;
};

struct game_state {
	platform_api* api = NULL;
	
	arena_allocator 	transient_arena;
	platform_allocator 	default_platform_allocator;
	platform_allocator  suppressed_platform_allocator;

	bool running = false;

	log_manager log;
	ogl_manager ogl;
	gui_manager gui;
	dbg_manager dbg;
	evt_manager evt;
	threadpool  thread_pool;

	platform_allocator log_a;
	platform_allocator ogl_a;
	platform_allocator gui_a;
	platform_allocator dbg_a;
	platform_allocator evt_a;
	platform_allocator thread_pool_a; // idk about this

	platform_window window 	= {};
	i32 window_w = 0;
	i32 window_h = 0;

	// testing
	asset_store default_store;
	platform_allocator default_store_a;
};
