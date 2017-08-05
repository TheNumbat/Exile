
dbg_manager make_dbg_manager(log_manager* log, allocator* alloc) { PROF

	dbg_manager ret;

	ret.log_cache = make_vector<log_message>(1024, alloc);
	ret.alloc = alloc;

	log_out dbg_log;
	dbg_log.level = log_level::info;
	dbg_log.custom = true;
	dbg_log.write = &dbg_add_log;
	logger_add_output(log, dbg_log);

	return ret;
}

void destroy_dbg_manager(dbg_manager* dbg) { PROF

	FORVEC(dbg->log_cache,
		DESTROY_ARENA(&it->arena);
	)

	destroy_vector(&dbg->log_cache);
}

void dbg_add_log(log_message* msg) { PROF

	// TODO(max): circular buffer
	if(global_dbg->log_cache.size == global_dbg->log_cache.capacity) {

		log_message* m = vector_front(&global_dbg->log_cache);
		DESTROY_ARENA(&m->arena);
		vector_pop_front(&global_dbg->log_cache);
	}

	log_message* m = vector_push(&global_dbg->log_cache, *msg);
	m->arena       = MAKE_ARENA("cmsg", msg->arena.size, global_dbg->alloc, msg->arena.suppress_messages);
	m->call_stack  = array<code_context>::make_copy(&msg->call_stack, &m->arena);
	m->thread_name = make_copy_string(msg->thread_name, &m->arena);
	m->msg         = make_copy_string(msg->msg, &m->arena);
}

void render_debug_gui(platform_window* win, dbg_manager* dbg) { PROF

	gui_begin(string_literal("Debug"));
	gui_log_wnd(win, string_literal("Log"), &dbg->log_cache);
}
