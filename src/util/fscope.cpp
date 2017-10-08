
#ifdef DO_PROF
func_scope::func_scope(code_context context) {
	LOG_DEBUG_ASSERT(this_thread_data.call_stack_depth < MAX_CALL_STACK_DEPTH);

	this_thread_data.call_stack[this_thread_data.call_stack_depth++] = context;

	dbg_msg m;
	m.type = dbg_msg_type::enter_func;
	m.enter_func.func = context;
	
	this_thread_data.dbg_msgs.push_noprof(m);
}

func_scope::~func_scope() {
	this_thread_data.call_stack_depth--;

	dbg_msg m;
	m.type = dbg_msg_type::exit_func;

	this_thread_data.dbg_msgs.push_noprof(m);
}

func_scope_nocs::func_scope_nocs(code_context context) {

	dbg_msg m;
	m.type = dbg_msg_type::enter_func;
	m.enter_func.func = context;

	this_thread_data.dbg_msgs.push_noprof(m);
}

func_scope_nocs::~func_scope_nocs() {
	
	dbg_msg m;
	m.type = dbg_msg_type::exit_func;

	this_thread_data.dbg_msgs.push_noprof(m);
}
#endif
