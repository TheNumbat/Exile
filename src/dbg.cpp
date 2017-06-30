
dbg_manager make_dbg_manager(allocator* alloc) { FUNC

	dbg_manager ret;

	ret.log_cache = make_vector<log_message>(1024, alloc);
	ret.alloc = alloc;

	log_out dbg_log;
	dbg_log.level = log_level::debug;
	dbg_log.custom = true;
	dbg_log.write = &dbg_add_log;
	logger_add_output(&global_state->log, dbg_log);

	return ret;
}

void destroy_dbg_manager(dbg_manager* dbg) { FUNC

	FORVEC(dbg->log_cache,
		destroy_array(&it->call_stack);
		free_string(it->thread_name, dbg->alloc);
		free_string(it->msg, dbg->alloc);
	)

	destroy_vector(&dbg->log_cache);
}

void dbg_add_log(log_message* msg) { FUNC

	dbg_manager* dbg = &global_state->dbg;

	// TODO(max): circular buffer
	if(dbg->log_cache.size == dbg->log_cache.capacity) {

		log_message* m = vector_front(&dbg->log_cache);
		destroy_array(&m->call_stack);
		free_string(m->thread_name, dbg->alloc);
		free_string(m->msg, dbg->alloc);
		vector_pop_front(&dbg->log_cache);
	}

	log_message m = *msg;
	m.call_stack = make_copy_array(&msg->call_stack, dbg->alloc);
	m.thread_name = make_copy_string(msg->thread_name, dbg->alloc);
	m.msg = make_copy_string(msg->msg, dbg->alloc);
	vector_push(&dbg->log_cache, m);
}

void render_debug_gui(game_state* state) { FUNC

	gui_begin(string_literal("Debug"));
	gui_log_wnd(string_literal("Log"), &state->dbg.log_cache);
}
