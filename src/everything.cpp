
inline code_context make_context(string file, string function, i32 line) {

	code_context ret;
	ret.file = file;
	ret.function = function;
	ret.line = line;

	return ret;
}
