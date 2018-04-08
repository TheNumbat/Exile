
string make_string_from_c_str(char* c_str, void* (*allocator)(u64 bytes)) {

	string ret;

	u32 len;
	for(len = 0; c_str[len] != '\0'; len++);
	len++;

	ret.c_str = (char*)(*allocator)(len);
	ret.cap = len ;
	ret.len = len ;

	for(u32 i = 0; i < len; i++) { // will copy null terminator
		ret.c_str[i] = c_str[i];
	}

	return ret;
}

string str(const char* c_str) {
	return string_from_c_str((char*)c_str);
}

string string_from_c_str(char* c_str) {

	string ret;

	u32 len;
	for(len = 0; c_str[len] != '\0'; len++);
	len++;

	ret.c_str = c_str;
	ret.len = len;
	ret.cap = len;

	return ret;
}

void free_string(string s, void	(*platform_heap_free)(void* mem)) {

	(*platform_heap_free)(s.c_str);

	s.c_str = null;
	s.cap = 0;
	s.len = 0;
}

string make_string(u32 cap, void* (*allocator)(u64 bytes)) {

	string ret;

	ret.c_str = (char*)(*allocator)(cap);
	ret.cap = cap;

	return ret;
}

string make_copy_string(string src, void* (*allocator)(u64 bytes)) {

	string ret = make_string(src.cap, allocator);

	ret.len = src.len;

	for(u32 i = 0; i <= ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

string string_literal(const char* literal) {

	return string_from_c_str((char*)literal);
}

i32 string_last_slash(string str) {

	for(u32 i = str.len; i >= 0; i--) {
		if(str.c_str[i] == '\\' || str.c_str[i] == '/') {
			return i;
		}
	}

	return -1;
}

// end exclusive
string make_substring(string str, u32 start, u32 end, void* (*allocator)(u64 bytes)) {

	string ret = make_string(end - start + 1, allocator);

	ret.len = end - start + 1;

	for(u32 i = 0, s_i = start; s_i < end; i++, s_i++) {
		ret.c_str[i] = str.c_str[s_i];
	}

	ret.c_str[ret.len - 1] = '\0';

	return ret;
}

string make_cat_string(string first, string second, void* (*allocator)(u64 bytes)) {

	string ret = make_string(first.len + second.len - 1, allocator);

	ret.len = first.len + second.len - 1;

	u32 c_i = 0;

	for(u32 i = 0; i < first.len - 1; i++, c_i++) {
		ret.c_str[c_i] = first.c_str[i];
	}

	for(u32 i = 0; i < second.len; i++, c_i++) {
		ret.c_str[c_i] = second.c_str[i];
	}

	return ret;
}
