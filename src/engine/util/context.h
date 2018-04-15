
#pragma once

#define CTX_FILE_LEN	32
#define CTX_FUN_LEN		128

struct code_context {
	char c_file[CTX_FILE_LEN]		= {};
	char c_function[CTX_FUN_LEN]	= {};
	u32 line = 0;

	string file();
	string function();
};

inline u32 hash(code_context c);
bool operator==(code_context l, code_context r);

#define CONTEXT _make_context(string::literal(__FILE__), string::literal(__FUNCNAME__), __LINE__)
inline code_context _make_context(string file, string function, i32 line);
