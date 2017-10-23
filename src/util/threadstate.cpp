
template<typename... Targs>
void _begin_thread(string fmt, allocator* alloc, u32 frames, u32 frame_size, code_context start, Targs... args) {

	make_type_table(alloc);
	this_thread_data.alloc_stack = stack<allocator*>::make(8, alloc);

	this_thread_data.dbg_msgs = queue<dbg_msg>::make(1024, alloc);
	global_dbg->register_thread(frames, frame_size);

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
