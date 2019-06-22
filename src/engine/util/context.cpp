
#include "context.h"
#include "../ds/map.h"
#include "../log.h"

inline u32 hash(code_context c) { 

	return hash(c.function()) ^ hash(c.file()) ^ hash(c.line);
}

bool operator==(code_context l, code_context r) { 

	return l.function() == r.function() && l.file() == r.file() && l.line == r.line;
}

string code_context::file() { 
	string f = path();
	return f.len > 1 ? f.substring(f.last_slash() + 1, f.len - 1) : f;
}
string code_context::function() { 
	return string::from_c_str(c_function);
}
string code_context::path() {
	return string::from_c_str(c_file);
}

inline code_context _make_context(string file, string function, i32 line) {

	code_context ret;

	LOG_DEBUG_ASSERT(file.len < CTX_FILE_LEN && function.len < CTX_FUN_LEN);

	_memcpy(file.c_str, ret.c_file, file.len);
	_memcpy(function.c_str, ret.c_function, function.len);
	ret.line = line;

	return ret;
}

