
#ifdef DO_PROF
func_scope::func_scope(code_context context) {
	LOG_DEBUG_ASSERT(this_thread_data.call_stack_depth < MAX_CALL_STACK_DEPTH);

	this_thread_data.call_stack[this_thread_data.call_stack_depth++] = context;

	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::enter_func;
		m.context = context;
	
		PUSH_PROFILE(false) {
			this_thread_data.dbg_msgs.push(m);
		} POP_PROFILE();
	}
}

func_scope::~func_scope() {
	this_thread_data.call_stack_depth--;

	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::exit_func;

		PUSH_PROFILE(false) {
			this_thread_data.dbg_msgs.push(m);
		} POP_PROFILE();
	}
}

func_scope_nocs::func_scope_nocs(code_context context) {

	if(this_thread_data.profiling) {	
		dbg_msg m;
		m.type = dbg_msg_type::enter_func;
		m.context = context;

		PUSH_PROFILE(false) {
			this_thread_data.dbg_msgs.push(m);
		} POP_PROFILE();
	}
}

func_scope_nocs::~func_scope_nocs() {
	
	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::exit_func;

		PUSH_PROFILE(false) {
			this_thread_data.dbg_msgs.push(m);
		} POP_PROFILE();
	}
}
#endif
