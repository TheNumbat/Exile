
inline code_context _make_context(string file, string function, i32 line) {

	code_context ret;
#ifdef PROFILE
	ret.file = file.substring(file.last_slash() + 1, file.len - 1);
	ret.function = function;
	ret.line = line;
#endif
	
	return ret;
}
