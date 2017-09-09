
inline code_context _make_context(string file, string function, i32 line) {

	code_context ret;
#ifdef DO_PROF
	ret.file = np_substring(file, np_string_last_slash(file) + 1, file.len - 1);
	ret.function = function;
	ret.line = line;
#endif
	
	return ret;
}
