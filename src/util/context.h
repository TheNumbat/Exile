
#pragma once

struct code_context {
	string file;
	string function;
	u32 line = 0;
};

#define CONTEXT _make_context(string::literal(__FILE__), string::literal(__FUNCNAME__), __LINE__)
inline code_context _make_context(string file, string function, i32 line);
