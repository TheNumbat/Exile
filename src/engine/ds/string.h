
#pragma once

#include "../basic.h"

struct allocator;
struct _type_info;

struct NOREFLECT string { // no-reflect because hard-coded
	char* c_str = null;
	u32 cap	    = 0;
	u32 len		= 0;	// including null terminator

///////////////////////////////////////////////////////////////////////////////

	// a lot of functions are a bit of a gray area regarding if they should
	// be a method...if it wasn't obvious that there is one "calling object"
	// I left them as free functions. (or if you need their signature like hash)

	static string make(u32 _cap);
	static string make(u32 _cap, allocator* a);
	static string make_copy(string src);
	static string make_copy(string src, allocator* a);
	static string make_copy_plt(string src);
	static string make_substring(string str, u32 start, u32 end);
	static string make_from_c_str(char* c_str); // copies
	static string make_from_c_str_plt(char* c_str); // copies
	static string make_cat(string first, string second);
	static string make_cat_v(i32 num_strs, ...);
	static string from_c_str(char* c_str); // does not allocate
	static string from_c_str(char* start, char* end); // does not allocate
	static string literal(const char* literal);

	template<typename... Targs> static string makef(string fmt, Targs... args);
	template<typename... Targs> static string makef(u32 len, string fmt, Targs... args);
	template<typename... Targs> static string makef(string fmt, allocator* a, Targs... args);

	operator const char*();
	operator char*();
	operator bool();

	void destroy(allocator* a);
	void destroy_plt();
	void destroy();

	// parsing stuff
	u32 get_next_codepoint(u32* index);
	i32 parse_i32(u32 idx = 0, u32* used = null);
	u32 parse_u32(u32 idx = 0, u32* used = null);

	string substring(u32 start, u32 end); 	// uses same memory
	i32 last_slash();
	i32 first_slash();

	string first_word_no_term();
	string trim_first_word();
	bool starts_with_insensitive(string prefix);
	string trim_new_term();
	string trim_copy();

	template<typename... Targs> void writef(string fmt, Targs... args);

	u32 write(u32 idx, string ins, bool size = false);
	u32 write(u32 idx, char ins, bool size = false);
	u32 write_type(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_queue(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_vector(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_heap(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_array(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_static_array(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_map(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_ptr(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_struct(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_any_struct(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_enum(u32 idx, void* val, _type_info* info, bool size = false);
	u32 write_int(u32 idx, u8 base, void* val, _type_info* info, bool size = false);
	u32 write_float(u32 idx, u8 precision, void* val, _type_info* info, bool size = false);
	u32 write_u64(u32 idx, u8 base, u64 val, bool size = false, u32 min_len = 0);
};

string operator "" _(const char* str, size_t s);
char uppercase(char c);
bool whitespace(char c);

inline u32 hash(string one, string two);
inline u32 hash(string str);
constexpr u32 const_hash(const char* str);
bool operator==(string first, string second);
inline bool operator==(string first, const char* second);
inline bool operator==(const char* first, string second);
bool strcmp(string first, string second);
bool operator<=(string first, string second);

// printf implementation

template<typename... Targs>
u32 size_stringf(string fmt, Targs... args);

// don't worry about these Kappa b

template<typename T, typename... Targs> 
inline T&  get_pack_first(T& val, Targs... args);
inline u32 get_pack_first();
template<typename T, typename... Targs>
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size, T val, Targs... args);
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size);

template<typename T, typename... Targs>
u32 _string_printf(string out, u32 idx, string fmt, bool size, T& value, Targs... args);
u32 _string_printf(string out, u32 idx, string fmt, bool size = false);

