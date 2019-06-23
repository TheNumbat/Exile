
#include "threadstate.h"
#include "reflect.h"
#include "context.h"

#include "../ds/string.h"
#include "../dbg.h"

thread_local thread_data this_thread_data;

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
