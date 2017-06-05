
#pragma once

struct game_state {
	platform_api* api = NULL;
	
	platform_mutex alloc_contexts_mutex;
	map<platform_thread_id, stack<allocator*>> alloc_contexts;
	
	platform_allocator default_platform_allocator;
	platform_allocator suppressed_platform_allocator;
	
	logger log;
	threadpool thread_pool;
	event_manager events;
	opengl ogl;

	i32 window_w = 0, window_h = 0;
	platform_window window 	= {};

	// testing
	asset_store test_store;
	texture_id 			texture;
	shader_program_id	shader;
	context_id			context;
};
