
CALLBACK u32 hash_context(code_context c) { PROF

	return hash_string(string::from_c_str(c.function)) ^ hash_string(string::from_c_str(c.file)) ^ hash_u32(c.line);
}

bool operator==(code_context l, code_context r) { PROF

	return string::from_c_str(l.function) == string::from_c_str(r.function) && string::from_c_str(l.file) == string::from_c_str(r.file) && l.line == r.line;
}

inline code_context _make_context(string file, string function, i32 line) {

	code_context ret;

#ifdef PROFILE
	file = file.substring(file.last_slash() + 1, file.len - 1);

	LOG_DEBUG_ASSERT(file.len < CTX_MAX_STR && function.len < CTX_MAX_STR);

	_memcpy_ctx(file.c_str, ret.file, file.len);
	_memcpy_ctx(function.c_str, ret.function, function.len);
	ret.line = line;
#endif
	
	return ret;
}

