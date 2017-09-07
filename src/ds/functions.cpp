

void func_ptr_state::reload_all() {
	FORMAP(all_ptrs,
		CHECKED(platform_get_proc_address, &it->value->data, global_func->this_dll, it->value->n);
	)
}

func_ptr_state func_ptr_state::make(allocator* a) {

	func_ptr_state ret;

	ret.all_ptrs = map<u32,_FPTR*>::make(256, a);

	return ret;
}

void func_ptr_state::destroy() {

	all_ptrs.destroy();
}
