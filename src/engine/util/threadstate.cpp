
template<typename... Targs>
void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args) {

	make_type_table(alloc);
	this_thread_data.alloc_stack = stack<allocator*>::make(8, alloc);

	this_thread_data.dbg_msgs = queue<dbg_msg>::make(1024, alloc);
	this_thread_data.start_context = start;
	PUSH_ALLOC(alloc);
	this_thread_data.name = string::makef(fmt, args...);

	this_thread_data.profiling = true;
}

void end_thread() { 
	this_thread_data.profiling = false;
	
	this_thread_data.dbg_msgs.destroy();
	this_thread_data.name.destroy();
	POP_ALLOC();
	this_thread_data.alloc_stack.destroy();
	type_table.destroy();
}
