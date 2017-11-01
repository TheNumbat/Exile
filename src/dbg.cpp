
dbg_manager dbg_manager::make(allocator* alloc) { PROF 

	dbg_manager ret;

	ret.dbg_cache = map<platform_thread_id,thread_profile>::make(global_api->platform_get_num_cpus(), alloc);
	ret.log_cache = queue<log_message>::make(1024, alloc);

	ret.alloc = alloc;
	ret.scratch = MAKE_ARENA(string::literal("dbg scratch"), MEGABYTES(1), alloc, false);

	global_api->platform_create_mutex(&ret.cache_mut, false);

	return ret;
}

void dbg_manager::destroy() { PROF

	FORMAP(it, dbg_cache) {

		FORQ_BEGIN(f, it->value.frames) {
			DESTROY_POOL(&f->pool);
		} FORQ_END(f, it->value.frames);

		it->value.frames.destroy();
	}

	FORQ_BEGIN(it, log_cache) {
		DESTROY_ARENA(&it->arena);
	} FORQ_END(it, log_cache);

	log_cache.destroy();

	global_api->platform_destroy_mutex(&cache_mut);

	dbg_cache.destroy();

	DESTROY_ARENA(&scratch);
}

void dbg_manager::UI() { PROF

	v2  dim = gui_window_dim();
	f32 height = 300.0f, width = 250.0f;

	gui_begin(string::literal("Debug"), R2(20.0f, 20.0f, width, height));

	gui_begin_list(string::literal("Log"));
	for(u32 i = 0; i < log_cache.len(); i++) {
		
		log_message* it = log_cache.get(i);

		PUSH_ALLOC(&scratch) {
			
			string level = it->fmt_level();

			string fmt = string::makef(string::literal("[%] %"), level, it->msg);
			gui_text(fmt);

			fmt.destroy();

		} POP_ALLOC();
		RESET_ARENA(&scratch);
	}
	gui_end_list();

	gui_end();
}

void dbg_manager::shutdown_log(log_manager* log) { PROF
	log_out dbg_log;
	dbg_log.level = log_level::debug;
	dbg_log.type = log_out_type::custom;
	dbg_log.write.set(FPTR(dbg_add_log));
	dbg_log.param = this;
	log->rem_custom_output(dbg_log);
}

void dbg_manager::setup_log(log_manager* log) { PROF
	log_out dbg_log;
	dbg_log.level = log_level::debug;
	dbg_log.type = log_out_type::custom;
	dbg_log.write.set(FPTR(dbg_add_log));
	dbg_log.param = this;
	log->add_custom_output(dbg_log);
}

void dbg_manager::register_thread(u32 frames, u32 frame_size) { PROF

	global_api->platform_aquire_mutex(&cache_mut);
	
	thread_profile thread;
	thread.frame_buf_size = frames;
	thread.frame_size = frame_size;
	thread.frames = queue<frame_profile>::make(frames, alloc);

	global_dbg->dbg_cache.insert(global_api->platform_this_thread_id(), thread);
	global_api->platform_release_mutex(&cache_mut);
}

void dbg_manager::collate() {

	PUSH_PROFILE(false) {
		global_api->platform_aquire_mutex(&cache_mut);
		thread_profile* thread = dbg_cache.get(global_api->platform_this_thread_id());
		global_api->platform_release_mutex(&cache_mut);

		FORQ_BEGIN(msg, this_thread_data.dbg_msgs) {

			if(msg->type == dbg_msg_type::begin_frame) {
			
				if(thread->frames.full()) {
					frame_profile rem = thread->frames.pop();
					DESTROY_POOL(&rem.pool);
				}
				frame_profile* frame = thread->frames.push(frame_profile());

				string name = string::makef(string::literal("frame %"), thread->num_frames);
				frame->pool = MAKE_POOL(name, KILOBYTES(8), alloc, false);
				frame->heads = vector<func_profile_node*>::make(2, &frame->pool);
		
				thread->num_frames++;
				name.destroy();
			}

#define NEW_NODE (new ((func_profile_node*)malloc(sizeof(func_profile_node))) func_profile_node)

			if(thread->num_frames && thread->frames.len() >= thread->num_frames % thread->frame_buf_size) {

				frame_profile* frame = thread->frames.get((thread->num_frames - 1) % thread->frame_buf_size);

				PUSH_ALLOC(&frame->pool) {
					switch(msg->type) {
					case dbg_msg_type::enter_func: {

						if(!frame->current) { 	
							frame->current = *frame->heads.push(NEW_NODE);
							frame->current->children = vector<func_profile_node*>::make(4);
							frame->current->context = msg->context;
							frame->current->current = msg->time;
							frame->current->calls++;
							break;
						}

						func_profile_node* here = frame->current;
						FORVEC(node, here->children) {
							if((*node)->context.function == msg->context.function) {
								here = *node;
								break;
							} 
						}
						if(here == frame->current) {
							func_profile_node* new_node = *here->children.push(NEW_NODE);
							new_node->children = vector<func_profile_node*>::make(4);
							new_node->parent = here;
							here = new_node;
						}
						
						here->context = msg->context;
						here->calls++;
						here->current = msg->time;
						frame->current = here;

					} break;

					case dbg_msg_type::exit_func: {

						frame->current->heir += msg->time - frame->current->current;
						
						frame->current->self += frame->current->heir;
						timestamp children = 0;
						FORVEC(it, frame->current->children) {
							children += (*it)->heir;
						}
						frame->current->self -= children;

						frame->current->current = 0;
						frame->current = frame->current->parent;

					} break;

					case dbg_msg_type::end_frame: {
						frame->end = msg->time;
					} break;
					}
				} POP_ALLOC();
			}
		} FORQ_END(msg, this_thread_data.dbg_msgs);

#undef NEW_NODE

		this_thread_data.dbg_msgs.clear();

	} POP_PROFILE();
}

CALLBACK void dbg_add_log(log_message* msg, void* param) { PROF

	dbg_manager* gui = (dbg_manager*)param;

	if(gui->log_cache.len() == gui->log_cache.capacity) {

		log_message* m = gui->log_cache.front();
		DESTROY_ARENA(&m->arena);
		gui->log_cache.pop();
	}

	log_message* m = gui->log_cache.push(*msg);
	m->arena       = MAKE_ARENA(string::literal("cmsg"), msg->arena.size, gui->alloc, msg->arena.suppress_messages);
	m->call_stack  = array<code_context>::make_copy(&msg->call_stack, &m->arena);
	m->thread_name = string::make_copy(msg->thread_name, &m->arena);
	m->msg         = string::make_copy(msg->msg, &m->arena);
}
