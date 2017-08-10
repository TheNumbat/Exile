
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

	for(u32 i = str.len - 1; i >= 0; i--) {
		if(str.c_str[i] == '\\' || str.c_str[i] == '/') {
			return i;
		}
	}

	return -1;
}

i32 np_string_first(string str, char c) { 

	for(u32 i = 0; i < str.len; i++) {
		if(str.c_str[i] == c) {
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
#ifdef DO_PROF
	ret.file = np_substring(file, np_string_last_slash(file) + 1, file.len - 1);
	
	i32 func_name_end = np_string_first(function, '(');
	i32 func_name_begin = func_name_end;
	while(function.c_str[func_name_begin--] != ' ');
	ret.function = np_substring(function, func_name_begin + 2, func_name_end);

	ret.line = line;
#endif
	
	return ret;
}

template<typename... Targs>
void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args) { PROF
	make_type_table(alloc);
	this_thread_data.alloc_stack = stack<allocator*>::make(8, alloc);
	this_thread_data.start_context = start;
	PUSH_ALLOC(alloc);
	this_thread_data.name = string::makef(fmt, args...);
}

void end_thread() { PROF
	this_thread_data.name.destroy();
	POP_ALLOC();

	this_thread_data.alloc_stack.destroy();
	destroy_type_table();
}
