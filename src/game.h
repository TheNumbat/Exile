
#pragma once

#include "common.h"
#include "threadpool.h"

struct game_state {
	platform_api* 	api 	= NULL;
	platform_window window 	= {};

	threadpool 	thread_pool;

	// TODO(max): thread-local
	stack<allocator*> global_alloc_context_stack; // global, but we can't have dll-local globals
	
	platform_allocator default_platform_allocator;
};
