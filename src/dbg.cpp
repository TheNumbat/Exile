
dbg_manager dbg_manager::make(allocator* alloc) { PROF 

	dbg_manager ret;

	ret.log_cache = queue<log_message>::make(1024, alloc);
	ret.dbg_cache = map<platform_thread_id,thread_profile>::make(global_api->platform_get_num_cpus(), alloc);
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

	FORQ(it, log_cache,
		DESTROY_ARENA(&it->arena);
	);
	LOG_INFO("LSJDFKL:SJDLKF:JDSLKF");
	FORMAP(it, dbg_cache,

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

		FORQ(f, it->value.frames,
			DESTROY_ARENA(&f->arena);
		)
		it->value.frames.destroy();
	);
	global_api->platform_destroy_mutex(&cache_mut);

	log_cache.destroy();
	dbg_cache.destroy();
}

void dbg_manager::register_thread(u32 frames, u32 frame_size) { PROF

	global_api->platform_aquire_mutex(&cache_mut);
	
	thread_profile thread;
	thread.frames = queue<frame_profile>::make(frames, alloc);
	thread.frame_size = frame_size;

	global_dbg->dbg_cache.insert(global_api->platform_this_thread_id(), thread);
	global_api->platform_release_mutex(&cache_mut);
}

void dbg_manager::collate() { PROF

	PUSH_PROFILE(false) {
		global_api->platform_aquire_mutex(&cache_mut);
		thread_profile* thread = dbg_cache.get(global_api->platform_this_thread_id());
		global_api->platform_release_mutex(&cache_mut);

		PUSH_ALLOC(alloc) {

			FORQ(it, this_thread_data.dbg_msgs,

				if(it->type == dbg_msg_type::begin_frame) {
					frame_profile frame;

					string name = string::makef(string::literal("frame %"), thread->num_frames);
					frame.arena = MAKE_ARENA_FROM_CONTEXT(name, thread->frame_size * sizeof(dbg_msg), false);
					name.destroy();
				
					frame_profile overwritten = thread->frames.push_overwrite(frame);
					LOG_INFO_F("%", overwritten);
					if(overwritten.arena.memory)
						DESTROY_ARENA(&overwritten.arena);
				}
			);
			this_thread_data.dbg_msgs.clear();

		} POP_ALLOC();
	} POP_PROFILE();
}

CALLBACK void dbg_add_log(log_message* msg) { PROF

	if(global_dbg->log_cache.len() == global_dbg->log_cache.capacity) {

		log_message* m = global_dbg->log_cache.front();
		DESTROY_ARENA(&m->arena);
		global_dbg->log_cache.pop();
	}

	log_message* m = global_dbg->log_cache.push(*msg);
	m->arena       = MAKE_ARENA(string::literal("cmsg"), msg->arena.size, global_dbg->alloc, msg->arena.suppress_messages);
	m->call_stack  = array<code_context>::make_copy(&msg->call_stack, &m->arena);
	m->thread_name = string::make_copy(msg->thread_name, &m->arena);
	m->msg         = string::make_copy(msg->msg, &m->arena);
}

void dbg_manager::render_debug_gui(platform_window* win) { PROF

	gui_begin(string::literal("Debug"));
	gui_log_wnd(win, string::literal("Log"), &log_cache);
}
