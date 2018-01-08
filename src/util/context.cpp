
CALLBACK u32 hash_context(code_context c) { PROF

	return hash_string(c.function) ^ hash_string(c.file) ^ hash_u32(c.line);
}

bool operator==(code_context l, code_context r) { PROF

	return l.function == r.function && l.file == r.file && l.line == r.line;
}

inline code_context _make_context(string file, string function, i32 line) {

	code_context ret;
#ifdef PROFILE
	ret.file = file.substring(file.last_slash() + 1, file.len - 1);
	ret.function = function;
	ret.line = line;
#endif
	
	return ret;
}
