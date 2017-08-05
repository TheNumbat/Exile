
#pragma once

struct NOREFLECT string { // no-reflect because hard-coded
	char* c_str = null;
	u32 cap	    = 0;
	u32 len		= 0;	// including null terminator
};

u32  hash_strings(string one, string two);
u32  hash_string(string str);
bool operator==(string first, string second);
inline bool operator==(string first, const char* second);
inline bool operator==(const char* first, string second);
bool strcmp(string first, string second);

// parsing stuff
u32 get_next_codepoint(string text_utf8, u32* index);
u32 parse_u32(string s, u32 idx = 0, u32* used = null);

// string utilities
string make_string_from_c_str(char* c_str); 		// copies
void   free_string(string s);
string make_string(u32 cap);
string make_copy_string(string src);
string make_substring(string str, u32 start, u32 end);
string substring(string str, u32 start, u32 end); 	// uses same memory
i32    string_last_slash(string str); 				// this returns an i32. just use this on non-2GB strings...
string make_cat_string(string first, string second);
string make_cat_strings(i32 num_strs, ...);
inline string string_literal(const char* literal);
string string_from_c_str(char* c_str); 				// does not allocate
u32    string_write(string s, u32 idx, string ins, bool size = false);
u32    string_write(string s, u32 idx, char ins, bool size = false);

struct 	allocator;
string 	make_string(u32 cap, allocator* a);
string 	make_copy_string(string src, allocator* a);
void 	free_string(string s, allocator* a);

// formating API
template<typename... Targs> string 	make_stringf(string fmt, Targs... args);
template<typename... Targs> string 	make_stringf_len(u32 len, string fmt, Targs... args);
template<typename... Targs> u32 	size_stringf(string fmt, Targs... args);
template<typename... Targs> void 	string_printf(string out, string fmt, Targs... args);

// variadic template helpers
template<typename T, typename... Targs> 
inline T&  get_pack_first(T& val, Targs... args);
inline u32 get_pack_first();
template<typename T, typename... Targs>
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size, T val, Targs... args);
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size);

// printf implementation
template<typename T, typename... Targs>
u32 _string_printf(string out, u32 idx, string fmt, bool size, T& value, Targs... args);
u32 _string_printf(string out, u32 idx, string fmt, bool size = false);

struct _type_info;
// print any type (calls specific prints + recurs on structure)
u32 print_type(string s, u32 idx, void* val, _type_info* info, bool size = false);

// type-specific helpers
u32 print_queue(string s, u32 idx, void* val, _type_info* info, bool size = false);
u32 print_vector(string s, u32 idx, void* val, _type_info* info, bool size = false);
u32 print_array(string s, u32 idx, void* val, _type_info* info, bool size = false);
u32 print_static_array(string s, u32 idx, void* val, _type_info* info, bool size = false);
u32 print_map(string s, u32 idx, void* val, _type_info* info, bool size = false);
u32 print_ptr(string s, u32 idx, void* val, _type_info* info, bool size = false);
u32 print_struct(string s, u32 idx, void* val, _type_info* info, bool size = false);
u32 print_enum(string s, u32 idx, void* val, _type_info* info, bool size = false);
u32 print_int(string s, u32 idx, u8 base, void* val, _type_info* info, bool size = false);
u32 print_float(string s, u32 idx, u8 precision, void* val, _type_info* info, bool size = false);
u32 print_u64(string s, u32 idx, u8 base, u64 val, bool size = false, u32 min_len = 0);
i64 int_as_i64(void* val, _type_info* info);
