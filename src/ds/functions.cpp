

void func_ptr_state::reload_all() {
	for(u32 i = 0; i < current_func_ptrs; i++) {
		CHECKED(platform_get_proc_address, &all_func_ptrs[i]->data, global_func->this_dll, all_func_ptrs[i]->n);
	}
}
