
#pragma once

#include "../ds/stack.h"
#include "../ds/string.h"
#include "../ds/queue.h"
#include "../ds/alloc.h"
#include "../dbg.h"

#define MAX_CONTEXT_DEPTH 16

struct thread_data {
	stack<allocator*> alloc_stack;
	
	string name;
	code_context start_context;
	bool startup = true;

	// NOTE(max): only does log context now
	string context_stack[MAX_CONTEXT_DEPTH] = {};
	u32 context_depth = 0;

	queue<dbg_msg> dbg_queue;
	arena_allocator scratch_arena;
};

static thread_local thread_data this_thread_data;

#define begin_thread(fmt, a, ...) _begin_thread(fmt, a, CONTEXT, ##__VA_ARGS__);
template<typename... Targs> void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args);
void end_thread();
