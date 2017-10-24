
#ifdef DO_PROF
func_scope::func_scope(code_context context) {
	LOG_DEBUG_ASSERT(this_thread_data.call_stack_depth < MAX_CALL_STACK_DEPTH);

	this_thread_data.call_stack[this_thread_data.call_stack_depth++] = context;

	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::enter_func;
		m.context = context;
		m.time = global_api->platform_get_perfcount();
	
		POST_MSG(m);
	}
}

func_scope::~func_scope() {
	this_thread_data.call_stack_depth--;

	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::exit_func;
		m.time = global_api->platform_get_perfcount();

		POST_MSG(m);
	}
}

func_scope_nocs::func_scope_nocs(code_context context) {

	if(this_thread_data.profiling) {	
		dbg_msg m;
		m.type = dbg_msg_type::enter_func;
		m.context = context;
		m.time = global_api->platform_get_perfcount();

		POST_MSG(m);
	}
}

func_scope_nocs::~func_scope_nocs() {
	
	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::exit_func;
		m.time = global_api->platform_get_perfcount();

		POST_MSG(m);
	}
}
#endif
