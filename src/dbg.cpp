
dbg_manager dbg_manager::make(log_manager* log, allocator* alloc) { PROF

	dbg_manager ret;

	ret.log_cache = queue<log_message>::make(1024, alloc);
	ret.alloc = alloc;

	log_out dbg_log;
	dbg_log.level = log_level::info;
	dbg_log.type = log_out_type::custom;
	dbg_log.write.set(FPTR(dbg_add_log));
	log->add_output(dbg_log);

	return ret;
}

void dbg_manager::destroy() { PROF

	really_running = false;

	FORQ(log_cache,
		DESTROY_ARENA(&it->arena);
	)

	log_cache.destroy();
}

void dbg_manager::collate() { PROF


}

CALLBACK void dbg_add_log(log_message* msg) { PROF

	if(global_dbg->log_cache.len() == global_dbg->log_cache.capacity) {

		log_message* m = global_dbg->log_cache.front();
		DESTROY_ARENA(&m->arena);
		global_dbg->log_cache.pop();
	}

	log_message* m = global_dbg->log_cache.push(*msg);
	m->arena       = MAKE_ARENA("cmsg", msg->arena.size, global_dbg->alloc, msg->arena.suppress_messages);
	m->call_stack  = array<code_context>::make_copy(&msg->call_stack, &m->arena);
	m->thread_name = string::make_copy(msg->thread_name, &m->arena);
	m->msg         = string::make_copy(msg->msg, &m->arena);
}

void dbg_manager::render_debug_gui(platform_window* win) { PROF

	gui_begin(string::literal("Debug"));
	gui_log_wnd(win, string::literal("Log"), &log_cache);
}
