
inline code_context make_context(string file, string function, i32 line) {

	code_context ret;
	ret.file = substring(file, string_last_slash(file) + 1, file.len);
	ret.function = function;
	ret.line = line;

	return ret;
}
