
#pragma once

struct string {
	char* c_str = NULL;
	u32 cap	    = 0;	// capacity
	u32 len		= 0;	// including null terminator
};
struct allocator;

u32 hash_strings(string one, string two);
u32 hash_string(string str);
bool operator==(string first, string second);
bool strcmp(string first, string second);

// utf-8 stuff
u32 get_next_codepoint(string text_utf8, u32* index);

string make_string_from_c_str(char* c_str);
void free_string(string s);
string make_string(u32 cap);
string make_copy_string(string src);
string make_substring(string str, u32 start, u32 end);
string substring(string str, u32 start, u32 end); // uses same memory
i32    string_last_slash(string str); // yes this returns an i32. just use this on non-2GB strings...
string make_cat_string(string first, string second);
string make_cat_strings(i32 num_strs, ...);
string string_literal(const char* literal);
string string_from_c_str(char* c_str);
u32 string_insert(string s, u32 idx, string ins);

template<typename... Targs> string make_stringf(string fmt, Targs... args);
template<typename T, typename... Targs> u32 string_printf(string out, u32 idx, string fmt, T value, Targs... args);
u32 string_printf(string out, u32 idx, string fmt);

struct _type_info;
template<typename T> u32 print_type(string s, u32 idx, T& val, _type_info* info = NULL);

string make_string(u32 cap, allocator* a);
string make_copy_string(string src, allocator* a);
void free_string(string s, allocator* a);
