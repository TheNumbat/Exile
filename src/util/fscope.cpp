
#ifdef PROFILE
func_scope::func_scope(code_context context) {
	LOG_DEBUG_ASSERT(this_thread_data.call_stack_depth < MAX_CALL_STACK_DEPTH);

	this_thread_data.call_stack[this_thread_data.call_stack_depth++] = context;

	if(this_thread_data.profiling && this_thread_data.timing_override) {
		dbg_msg m;
		m.type = dbg_msg_type::enter_func;
		m.context = context;
	
		POST_MSG(m);
	}
}

func_scope::~func_scope() {
	this_thread_data.call_stack_depth--;

	if(this_thread_data.profiling && this_thread_data.timing_override) {
		dbg_msg m;
		m.type = dbg_msg_type::exit_func;

		POST_MSG(m);
	}
}

func_scope_nocs::func_scope_nocs(code_context context) {

	if(this_thread_data.profiling && this_thread_data.timing_override) {	
		dbg_msg m;
		m.type = dbg_msg_type::enter_func;
		m.context = context;

		POST_MSG(m);
	}
}

func_scope_nocs::~func_scope_nocs() {
	
	if(this_thread_data.profiling && this_thread_data.timing_override) {
		dbg_msg m;
		m.type = dbg_msg_type::exit_func;

		POST_MSG(m);
	}
}
#endif
