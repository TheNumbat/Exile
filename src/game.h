
#pragma once

struct game_state {
	// stuff available globally
	platform_api* api = NULL;
	platform_mutex alloc_contexts_mutex;
	map<platform_thread_id, stack<allocator*>> alloc_contexts;
	platform_allocator default_platform_allocator;
	platform_allocator suppressed_platform_allocator;
	logger log;
	threadpool thread_pool;

	// don't touch these globally
	platform_window window 	= {};
};
