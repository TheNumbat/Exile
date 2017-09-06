
template<typename T, typename... args>
func_ptr<T, args...>::func_ptr() {
	global_func->ptrs_existing++;
}

template<typename T, typename... args>
func_ptr<T, args...>::~func_ptr() {
	global_func->ptrs_existing--;
}

template<typename T, typename... args>
void func_ptr<T, args...>::set(T (*f)(args...), string n) {
	func = f;
	name = n;
}

template<typename T, typename... args>
void func_ptr<T, args...>::reload() {
	CHECKED(platform_get_proc_address, (void**)&func, global_func->this_dll, name);
}

CALLBACK void test() {
	LOG_INFO("ULU");
}

void test_funcs() {

	global_func->test.set(FPTR(test));
	global_func->test();
}
