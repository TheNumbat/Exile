
func_scope::func_scope(code_context context) {
	LOG_DEBUG_ASSERT(this_thread_data.call_stack_depth < MAX_CALL_STACK_DEPTH);
	LOG_DEBUG_ASSERT(this_thread_data.dbg_cache_size < MAX_DBG_MSG_CACHE);

	this_thread_data.call_stack[this_thread_data.call_stack_depth++] = context;

	dbg_msg m;
	m.type = dbg_msg_type::enter_func;
	m.enter_func.func = context;
	// this_thread_data.dbg_cache[this_thread_data.dbg_cache_size++] = m;
	// printf("%+*s\n", this_thread_data.call_stack_depth + context.function.len, context.function.c_str);
}

func_scope::~func_scope() {
	LOG_DEBUG_ASSERT(this_thread_data.dbg_cache_size < MAX_DBG_MSG_CACHE);

	dbg_msg m;
	m.type = dbg_msg_type::exit_func;
	// this_thread_data.dbg_cache[this_thread_data.dbg_cache_size++] = m;

	this_thread_data.call_stack_depth--;
}

func_scope_nocs::func_scope_nocs(code_context context) {

	LOG_DEBUG_ASSERT(this_thread_data.dbg_cache_size < MAX_DBG_MSG_CACHE);

	dbg_msg m;
	m.type = dbg_msg_type::enter_func;
	m.enter_func.func = context;
	// this_thread_data.dbg_cache[this_thread_data.dbg_cache_size++] = m;
}

func_scope_nocs::~func_scope_nocs() {
	
	LOG_DEBUG_ASSERT(this_thread_data.dbg_cache_size < MAX_DBG_MSG_CACHE);

	dbg_msg m;
	m.type = dbg_msg_type::exit_func;
	// this_thread_data.dbg_cache[this_thread_data.dbg_cache_size++] = m;
}
