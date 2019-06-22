
#include "threadstate.h"
#include "../ds/string.h"
#include "context.h"

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

void end_thread() { 

	this_thread_data.startup = true;
	
	this_thread_data.name.destroy(this_thread_data.alloc_stack.contents.alloc);
	type_table.destroy();

	DESTROY_ARENA(&this_thread_data.scratch_arena);
	POP_ALLOC();
	
	global_dbg->profiler.unregister_thread();
	
	this_thread_data.dbg_queue.destroy();
	this_thread_data.alloc_stack.destroy();
}
