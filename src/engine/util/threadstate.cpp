
template<typename... Targs>
void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args) {

	make_type_table(alloc);
	this_thread_data.alloc_stack = stack<allocator*>::make(8, alloc);

	this_thread_data.dbg_queue = queue<dbg_msg>::make(4096, alloc);

	this_thread_data.start_context = start;
	this_thread_data.name = string::makef(fmt, alloc, args...);

	this_thread_data.scratch_arena = MAKE_ARENA("scratch"_, MEGABYTES(32), alloc);
	PUSH_ALLOC(&this_thread_data.scratch_arena);

	this_thread_data.profiling = true;
}

void end_thread() { 

	DESTROY_ARENA(&this_thread_data.scratch_arena);

	this_thread_data.profiling = false;
	
	this_thread_data.dbg_queue.destroy();
 	
	this_thread_data.name.destroy(this_thread_data.alloc_stack.contents.alloc);
	POP_ALLOC();
	this_thread_data.alloc_stack.destroy();
	type_table.destroy();
}
