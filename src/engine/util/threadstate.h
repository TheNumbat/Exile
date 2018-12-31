

#pragma once

#define MAX_CALL_STACK_DEPTH 	256

struct thread_data {
	stack<allocator*> alloc_stack;
	
	string name;
	code_context start_context;
	bool startup = true;

	code_context call_stack[MAX_CALL_STACK_DEPTH] = {};
	u32 call_stack_depth = 0;

	queue<dbg_msg> dbg_queue;
	arena_allocator scratch_arena;
};

static thread_local thread_data this_thread_data;

#define begin_thread(fmt, a, ...) _begin_thread(fmt, a, CONTEXT, ##__VA_ARGS__);
template<typename... Targs> void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args);
void end_thread();
