
#pragma once

struct string { // @NORTTI - hardcoded
	char* c_str = NULL;
	u32 cap	    = 0;	// capacity
	u32 len		= 0;	// including null terminator
};

u32 hash_strings(string one, string two);
u32 hash_string(string str);
bool operator==(string first, string second);
bool strcmp(string first, string second);

// utf-8 stuff
u32 get_next_codepoint(string text_utf8, u32* index);

u32 parse_u32(string s, u32 idx = 0, u32* used = NULL);

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
u32 string_insert(string s, u32 idx, string ins, bool size = false);
u32 string_insert(string s, u32 idx, char ins, bool size = false);

// variadic template magic
template<typename T, typename... Targs> inline T& get_pack_first(T& val, Targs... args);
inline u32 get_pack_first();
template<typename T, typename... Targs> inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size, T val, Targs... args);
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size);

// more magic, but only for typed print
template<typename... Targs> string make_stringf(string fmt, Targs... args);
template<typename... Targs> u32 size_stringf(string fmt, Targs... args);

template<typename... Targs> void string_printf(string out, string fmt, Targs... args);
template<typename T, typename... Targs> u32 _string_printf(string out, u32 idx, string fmt, bool size, T value, Targs... args);
u32 _string_printf(string out, u32 idx, string fmt, bool size = false);

struct _type_info;
// type-specific prints
template<typename T> u32 print_int(string s, u32 idx, u8 base, T& val, _type_info* info, bool size = false);
template<typename T> u32 print_float(string s, u32 idx, T& val, _type_info* info, bool size = false);
u32 print_u64(string s, u32 idx, u8 base, u64 val, bool size = false);

// print any type (calls specific prints + recurs on structure)
template<typename T> u32 print_type(string s, u32 idx, T& val, _type_info* info = NULL, bool size = false);

struct allocator;
string make_string(u32 cap, allocator* a);
string make_copy_string(string src, allocator* a);
void free_string(string s, allocator* a);
