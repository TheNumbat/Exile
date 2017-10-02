
template<typename... Targs>
void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args) {
	make_type_table(alloc);
	this_thread_data.alloc_stack = stack<allocator*>::make(8, alloc);

	// global_dbg->dbg_cache.insert(global_api->platform_this_thread_id(), queue<dbg_msg>::make(1024, alloc));

	this_thread_data.start_context = start;
	PUSH_ALLOC(alloc);
	this_thread_data.name = string::makef(fmt, args...);
}

void end_thread() { 
	this_thread_data.name.destroy();
	POP_ALLOC();

	// global_dbg->dbg_cache.erase(global_api->platform_this_thread_id());

	this_thread_data.alloc_stack.destroy();
	type_table.destroy();
}
