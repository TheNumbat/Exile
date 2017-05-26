
#pragma once

struct game_state {
	platform_api* 	api 	= NULL;
	platform_window window 	= {};

	threadpool 	thread_pool;
	logger		log;

	platform_mutex alloc_contexts_mutex;
	map<platform_thread_id, stack<allocator*>> global_alloc_contexts;
	
	platform_allocator default_platform_allocator;
};
