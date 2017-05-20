
#pragma once

#include "common.h"

struct game_state {
	platform_api* 	api 	= NULL;
	platform_window window 	= {};

	// TODO(max): thread-local
	stack<allocator> global_alloc_context; // global, but we can't have dll-local globals
};
