
#pragma once

#include "../ds/stack.h"
#include "../ds/string.h"
#include "../ds/queue.h"
#include "../ds/alloc.h"
#include "../math.h"
#include "../util/reflect.h"

#define MAX_CONTEXT_DEPTH 16

struct dbg_msg;

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

extern thread_local thread_data this_thread_data;

#define begin_thread(fmt, a, ...) _begin_thread(fmt, a, CONTEXT, ##__VA_ARGS__);
template<typename... Targs> void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args);
void end_thread();

template<typename... Targs>
void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args) {

	this_thread_data.alloc_stack = stack<allocator*>::make(8, alloc);

	this_thread_data.dbg_queue = queue<dbg_msg>::make(4096, alloc);

	this_thread_data.scratch_arena = MAKE_ARENA("scratch"_, MEGABYTES(8), alloc);
	PUSH_ALLOC(&this_thread_data.scratch_arena);

	make_type_table(alloc);
	
	this_thread_data.start_context = start;
	this_thread_data.name = string::makef(fmt, alloc, args...);

	rand_init(hash(this_thread_data.name));
	
	this_thread_data.startup = false;
}

