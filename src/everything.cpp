
string np_string_from_c_str(char* c_str) {

	string ret;

	u32 len;
	for(len = 0; c_str[len] != '\0'; len++);
	len++;

	ret.c_str = c_str;
	ret.len = len;
	ret.cap = len;

	return ret;
}

string np_substring(string str, u32 start, u32 end) {

	string ret;

	ret.len = end - start + 1;
	ret.cap = end - start + 1;

	ret.c_str = str.c_str + start;

	return ret;
}

i32 np_string_last_slash(string str) { 

	for(u32 i = str.len; i >= 0; i--) {
		if(str.c_str[i] == '\\' || str.c_str[i] == '/') {
			return i;
		}
	}

	return -1;
}

string np_string_literal(const char* literal) {

	return np_string_from_c_str((char*)literal);
}

inline code_context _make_context(string file, string function, i32 line) {

	code_context ret;
#ifdef _DEBUG
	ret.file = np_substring(file, np_string_last_slash(file) + 1, file.len - 1);
	ret.function = function;
	ret.line = line;
#endif
	
	return ret;
}

template<typename... Targs>
void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args) { FUNC
	make_type_table(alloc);
	this_thread_data.alloc_stack = make_stack<allocator*>(8, alloc);
	this_thread_data.start_context = start;
	PUSH_ALLOC(alloc);
	this_thread_data.name = make_stringf(fmt, args...);
}

void end_thread() { FUNC
	free_string(this_thread_data.name);
	POP_ALLOC();

	destroy_stack(&this_thread_data.alloc_stack);
	destroy_type_table();
}
