
#pragma once

struct string {
	char* c_str = null;
	u32 cap	    = 0;	// capacity
	u32 len		= 0;	// including null terminator
};

string make_string_from_c_str(char* c_str, void* (*allocator)(u64 bytes));
string string_from_c_str(char* c_str);
string str(const char* c_str);
void   free_string(string s, void (*platform_heap_free)(void* mem));
string make_string(u32 cap, void* (*allocator)(u64 bytes));
string make_copy_string(string src, void* (*allocator)(u64 bytes));
string string_literal(const char* literal);
i32    string_last_slash(string str);
string make_substring(string str, u32 start, u32 end, void* (*allocator)(u64 bytes));
string make_cat_string(string first, string second, void* (*allocator)(u64 bytes));
bool streql(string l, string r);
