
inline code_context _make_context(string file, string function, i32 line) { FUNC

	code_context ret;
#ifdef _DEBUG
	ret.file = substring(file, string_last_slash(file) + 1, file.len);
	ret.function = function;
	ret.line = line;
#endif
	
	return ret;
}

void _begin_thread(string name, allocator* alloc, code_context start) { FUNC
	this_thread_data.name = name;
	this_thread_data.context_stack = make_stack<string>(32, alloc);
	this_thread_data.alloc_stack = make_stack<allocator*>(8, alloc);
	this_thread_data.start_context = start;
	PUSH_ALLOC(alloc);
}

void end_thread() { FUNC
	POP_ALLOC();
	destroy_stack(&this_thread_data.alloc_stack);
	destroy_stack(&this_thread_data.context_stack);
}
