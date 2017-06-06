
#pragma once

struct game_state {
	platform_api* api = NULL;
	
	platform_mutex alloc_contexts_mutex;
	map<platform_thread_id, stack<allocator*>> alloc_contexts;
	
	platform_allocator 	default_platform_allocator;
	platform_allocator 	suppressed_platform_allocator;
	arena_allocator 	transient_arena;

	logger log;
	threadpool thread_pool;
	event_manager events;
	opengl ogl;
	gui_manager gui;

	i32 window_w = 0, window_h = 0;
	platform_window window 	= {};

	// testing
	asset_store default_store;
};
