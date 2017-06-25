
dbg_manager make_dbg_manager(allocator* alloc) {

	dbg_manager ret;

	ret.log_cache = make_vector<cached_message>(1024, alloc);
	ret.alloc = alloc;

	log_out dbg_log;
	dbg_log.level = log_debug;
	dbg_log.custom = true;
	dbg_log.write = &dbg_add_log;
	logger_add_output(&global_state->log, dbg_log);

	return ret;
}

void destroy_dbg_manager(dbg_manager* dbg) {

	FORVEC(dbg->log_cache,
		free_string(it->fmt, dbg->alloc);
	)

	destroy_vector(&dbg->log_cache);
}

void dbg_add_log(log_message* msg, string fmt) {

	dbg_manager* dbg = &global_state->dbg;

	// TODO(max): circular buffer
	if(dbg->log_cache.size == dbg->log_cache.capacity) {

		cached_message* cm = vector_front(&dbg->log_cache);
		free_string(cm->fmt, dbg->alloc);
		vector_pop_front(&dbg->log_cache);
	}

	cached_message cm;
	cm.msg = *msg;
	cm.fmt = make_copy_string(fmt, dbg->alloc);
	vector_push(&dbg->log_cache, cm);
}

void render_debug_gui(game_state* state) {

	gui_begin(string_literal("Debug"));
	gui_log_wnd(string_literal("Log"), &state->dbg.log_cache);
}
