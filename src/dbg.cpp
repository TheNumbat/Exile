
dbg_manager make_dbg_manager(allocator* alloc) {

	dbg_manager ret;

	ret.log_cache = make_vector<log_message>(1024, alloc);
	ret.alloc = alloc;

	return ret;
}

void destroy_dbg_manager(dbg_manager* dbg) {

	destroy_vector(&dbg->log_cache);
}

void dbg_add_log(log_message* msg) {

	dbg_manager* dbg = &global_state->dbg;

	// TODO(max): circular buffer
	if(dbg->log_cache.size == dbg->log_cache.capacity) {
		vector_pop_front(&dbg->log_cache);
	}
	vector_push(&dbg->log_cache, *msg);
}

void render_debug_gui(game_state* state) {

	gui_begin(string_literal("Debug"));
	gui_log_dsp(string_literal("Log"), &state->dbg.log_cache);
}
