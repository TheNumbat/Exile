
inline code_context make_context(string file, string function, i32 line) {

	code_context ret;
#ifdef _DEBUG
	ret.file = substring(file, string_last_slash(file) + 1, file.len);
	ret.function = function;
	ret.line = line;
#endif

	return ret;
}
