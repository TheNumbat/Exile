
template<typename T>
func_ptr<T>::func_ptr() {
	global_func->ptrs_existing++;
}

template<typename T>
func_ptr<T>::~func_ptr() {
	global_func->ptrs_existing--;
}

template<typename T>
void func_ptr<T>::set(T* f, string n) {
	func = f;
	name = n;
}


template<typename T>
template<typename... args>
auto func_ptr<T>::operator()(args...) {
	if(global_func->reloading) {
		reload();
		global_func->ptrs_reloaded++;
		if(global_func->ptrs_reloaded == global_func->ptrs_existing) {
			global_func->reloading = false;
		}
	}

	return func(args...);
}

template<typename T>
void func_ptr<T>::reload() {
	CHECKED(platform_get_proc_address, (void**)&func, global_func->this_dll, name);
}

CALLBACK void test() {
	LOG_INFO("ULU");
}

void test_funcs() {

	global_func->test.set(FPTR(test));
	global_func->test();
}
