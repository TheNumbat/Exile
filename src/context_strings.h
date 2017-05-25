
#pragma once

#include "strings.h"

string make_string_from_c_str(char* c_str);
void free_string(string s);
string make_string(u32 cap);
string make_copy_string(string src);
string make_substring(string str, u32 start, u32 end);
string make_cat_string(string first, string second);

string make_string_from_c_str(char* c_str) {

	u32 len;
	for(len = 0; c_str[len] != '\0'; len++);

	string ret = make_string(len);
	ret.len = len;

	for(u32 i = 0; i <= len; i++) { // will copy null terminator
		ret.c_str[i] = c_str[i];
	}

	return ret;
}

void free_string(string s) {

	_free(s.c_str);

	s.c_str = NULL;
	s.cap = 0;
	s.len = 0;
}

string make_string(u32 cap) {

	string ret;

	ret.c_str = (char*)_malloc(cap);
	ret.cap = cap;

	return ret;
}

string make_copy_string(string src) {

	string ret = make_string(src.cap);

	ret.len = src.len;

	for(u32 i = 0; i <= ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

// end inclusive
string make_substring(string str, u32 start, u32 end) {

	string ret = make_string(end - start + 1);

	ret.len = end - start + 1;

	for(u32 i = 0, s_i = start; s_i <= end; i++, s_i++) {
		ret.c_str[i] = str.c_str[s_i];
	}

	return ret;
}

string make_cat_string(string first, string second) {

	string ret = make_string(first.len + second.len - 1);

	ret.len = first.len + second.len - 1;

	u32 c_i = 0;

	for(u32 i = 0; i < first.len; i++, c_i++) {
		ret.c_str[c_i] = first.c_str[i];
	}

	for(u32 i = 0; i <= second.len; i++, c_i++) {
		ret.c_str[c_i] = second.c_str[i];
	}

	return ret;
}