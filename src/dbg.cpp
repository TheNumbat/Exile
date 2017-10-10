
dbg_manager dbg_manager::make(allocator* alloc) { PROF 

	dbg_manager ret;

	ret.log_cache = queue<log_message>::make(1024, alloc);
	ret.dbg_cache = map<platform_thread_id,queue<dbg_msg>>::make(global_api->platform_get_num_cpus(), alloc);
	ret.alloc = alloc;

	global_api->platform_create_mutex(&ret.cache_mut, false);

	return ret;
}

void dbg_manager::setup_log(log_manager* log) { PROF
	log_out dbg_log;
	dbg_log.level = log_level::info;
	dbg_log.type = log_out_type::custom;
	dbg_log.write.set(FPTR(dbg_add_log));
	log->add_output(dbg_log);
}

void dbg_manager::destroy() { PROF

	FORQ(log_cache,
		DESTROY_ARENA(&it->arena);
	);
	FORMAP(dbg_cache,

		/* printing out all functions called in the last frame
		bool start = false;
		FORQ2(it->value,
			if(it2->type == dbg_msg_type::begin_frame)
				if(start) break;
				else start = true;
			if(start && it2->type == dbg_msg_type::enter_func) {
				std::cout << it2->context.function.c_str << std::endl;
			}
		)
		*/

		it->value.destroy();
	);
	global_api->platform_destroy_mutex(&cache_mut);

	log_cache.destroy();
	dbg_cache.destroy();
}

void dbg_manager::register_thread(u32 cache_size) { PROF

	global_api->platform_aquire_mutex(&cache_mut);
	global_dbg->dbg_cache.insert(global_api->platform_this_thread_id(), queue<dbg_msg>::make(cache_size, alloc));
	global_api->platform_release_mutex(&cache_mut);
}

void dbg_manager::collate() { PROF

	PUSH_PROFILE(false) {
		global_api->platform_aquire_mutex(&cache_mut);
		queue<dbg_msg>* q = dbg_cache.get(global_api->platform_this_thread_id());

		FORQ(this_thread_data.dbg_msgs,
			q->push_overwrite(*it);
		);
		this_thread_data.dbg_msgs.clear();

		global_api->platform_release_mutex(&cache_mut);
	} POP_PROFILE();
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
