
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

	// yeah yeah this should just be an atomic increment
	global_api->platform_aquire_mutex(&global_func->mut);
	u32 idx = global_func->num_ptrs;
	global_func->num_ptrs++;
	global_api->platform_release_mutex(&global_func->mut);

	global_func->all_ptrs[idx].func = func;
	global_func->all_ptrs[idx].name = string::make_copy_plt(name);

	return &global_func->all_ptrs[idx];
}

void cleanup_fptrs() {
	for(u32 i = 0; i < global_func->num_ptrs; i++) {
		global_func->all_ptrs[i].name.destroy_plt();
	}
}

void setup_fptrs() {
	global_api->platform_create_mutex(&global_func->mut, false);
}
