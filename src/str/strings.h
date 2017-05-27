
#pragma once

struct string {
	char* c_str = NULL;
	u32 cap	    = 0;	// capacity
	u32 len		= 0;	// including null terminator
};

string make_string_from_c_str(char* c_str);
void free_string(string s);
string make_string(u32 cap);
string make_copy_string(string src);
string make_substring(string str, u32 start, u32 end);
string make_cat_string(string first, string second);
string make_cat_strings(i32 num_strs, ...);
string string_literal(const char* literal);
string string_from_c_str(char* c_str);

// TODO(max): replace vsprintf
string make_stringf(string fmt, ...);
string make_vstringf(string fmt, va_list args);
