
void func_ptr_state::reload_all() {
	for(u32 i = 0; i < global_func->num_ptrs; i++) {
		CHECKED(platform_get_proc_address, &global_func->all_ptrs[i].func, global_func->this_dll, global_func->all_ptrs[i].name);
	}
}

_FPTR* _fptr(void* func, string name) {

	for(u32 i = 0; i < global_func->num_ptrs; i++) {
		if(global_func->all_ptrs[i].func == func)  {
			return &global_func->all_ptrs[i];
		}
	}

	global_func->all_ptrs[global_func->num_ptrs].func = func;
	global_func->all_ptrs[global_func->num_ptrs].name = string::make_copy_plt(name);

	global_func->num_ptrs++;
	return &global_func->all_ptrs[global_func->num_ptrs - 1];
}

void cleanup_fptrs() {
	for(u32 i = 0; i < global_func->num_ptrs; i++) {
		global_func->all_ptrs[i].name.destroy_plt();
	}
}
