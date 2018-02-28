
void func_ptr_state::reload_all() { PROF
	for(u32 i = 0; i < num_ptrs; i++) {
		CHECKED(platform_get_proc_address, &all_ptrs[i].func, this_dll, all_ptrs[i].name);
	}
}

_FPTR* FPTR_STR(string name) { PROF

	void* func = null;
	CHECKED(platform_get_proc_address, &func, global_func->this_dll, name);

	if(!func) return null;

	return _fptr(func, name);	
}

_FPTR* _fptr(void* func, string name) { PROF

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

void cleanup_fptrs() { PROF
	for(u32 i = 0; i < global_func->num_ptrs; i++) {
		global_func->all_ptrs[i].name.destroy_plt();
	}
	global_api->platform_destroy_mutex(&global_func->mut);
}

void setup_fptrs() { PROF
	global_api->platform_create_mutex(&global_func->mut, false);
}
