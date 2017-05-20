
#pragma once

// be wary of using code reloading with this, it's shared between platform and game

struct string {
	char* c_str = NULL;
	i32 cap	    = 0;	// capacity
	i32 len		= 0;	// including null terminator
};

string make_string_from_c_str(char* c_str, void* (*allocator)(u64 bytes));
string string_from_c_str(char* c_str);
void free_string(string s, void	(*platform_heap_free)(void* mem));
string make_string(i32 cap, void* (*allocator)(u64 bytes));
string make_copy_string(string src, void* (*allocator)(u64 bytes));
string string_literal(const char* literal);
i32 string_last_slash(string str);
string make_substring(string str, i32 start, i32 end, void* (*allocator)(u64 bytes));
string make_cat_string(string first, string second, void* (*allocator)(u64 bytes));

string make_string_from_c_str(char* c_str, void* (*allocator)(u64 bytes)) {

	string ret;

	i32 len;
	for(len = 0; c_str[len] != '\0'; len++);

	ret.c_str = (char*)(*allocator)(len);
	ret.cap = len;
	ret.len = len;

	for(i32 i = 0; i <= len; i++) { // will copy null terminator
		ret.c_str[i] = c_str[i];
	}

	return ret;
}

string string_from_c_str(char* c_str) {

	string ret;

	i32 len;
	for(len = 0; c_str[len] != '\0'; len++);

	ret.c_str = c_str;
	ret.len = len;
	ret.cap = len;

	return ret;
}

void free_string(string s, void	(*platform_heap_free)(void* mem)) {

	(*platform_heap_free)(s.c_str);

	s.c_str = NULL;
	s.cap = 0;
	s.len = 0;
}

string make_string(i32 cap, void* (*allocator)(u64 bytes)) {

	string ret;

	ret.c_str = (char*)(*allocator)(cap);
	ret.cap = cap;

	return ret;
}

string make_copy_string(string src, void* (*allocator)(u64 bytes)) {

	string ret = make_string(src.cap, allocator);

	ret.len = src.len;

	for(i32 i = 0; i <= ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

string string_literal(const char* literal) {

	return string_from_c_str((char*)literal);
}

i32 string_last_slash(string str) {

	for(i32 i = str.len; i >= 0; i--) {
		if(str.c_str[i] == '\\' || str.c_str[i] == '/') {
			return i;
		}
	}

	return -1;
}

// end inclusive
string make_substring(string str, i32 start, i32 end, void* (*allocator)(u64 bytes)) {

	string ret = make_string(end - start + 1, allocator);

	ret.len = end - start + 1;

	for(i32 i = 0, s_i = start; s_i <= end; i++, s_i++) {
		ret.c_str[i] = str.c_str[s_i];
	}

	return ret;
}

string make_cat_string(string first, string second, void* (*allocator)(u64 bytes)) {

	string ret = make_string(first.len + second.len - 1, allocator);

	ret.len = first.len + second.len - 1;

	i32 c_i = 0;

	for(i32 i = 0; i < first.len; i++, c_i++) {
		ret.c_str[c_i] = first.c_str[i];
	}

	for(i32 i = 0; i <= second.len; i++, c_i++) {
		ret.c_str[c_i] = second.c_str[i];
	}

	return ret;
}