
dbg_manager dbg_manager::make(allocator* alloc) { PROF 

	dbg_manager ret;

	ret.dbg_cache = map<platform_thread_id,thread_profile>::make(global_api->platform_get_num_cpus(), alloc);
	ret.log_cache = queue<log_message>::make(1024, alloc);

	ret.alloc = alloc;
	ret.scratch = MAKE_ARENA("dbg scratch"_, MEGABYTES(1), alloc, false);

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

void dbg_manager::profile_recurse(vector<func_profile_node*> list) { PROF

	switch(prof_sort) {
	case prof_sort_type::none: break;
	case prof_sort_type::name: {
		list.stable_sort(prof_sort_name);
	} break;
	case prof_sort_type::heir: {
		list.stable_sort(prof_sort_heir);
	} break;
	case prof_sort_type::self: {
		list.stable_sort(prof_sort_self);
	} break;
	case prof_sort_type::calls: {
		list.stable_sort(prof_sort_calls);
	} break;
	}

	FORVEC(it, list) {
		gui_push_id(__it);

		func_profile_node* node = *it;

		if(gui_node(string::makef("%--*|%+8|%+10|%+2"_, 35 - gui_indent_level(), node->context.function, node->heir, node->self, node->calls), &node->enabled)) {
			gui_indent();
			profile_recurse(node->children);
			gui_unindent();
		}

		gui_pop_id();
	}
}

void dbg_manager::UI() { PROF

	v2 dim = gui_window_dim();

	// NOTE(max): all the makef-ing here just comes from the scratch buffer

	gui_begin("Console"_, R2(0.0f, dim.y * 0.75f, dim.x, dim.y / 4.0f), (u16)window_flags::nowininput);

	PUSH_ALLOC(&scratch);
	FORQ_BEGIN(it, log_cache) {

			string level = it->fmt_level();
			gui_text(string::makef("[%-5] %"_, level, it->msg));
	
	} FORQ_END(it, log_cache);

	gui_end();

	gui_begin("Profile"_, R2(20.0f, 20.0f, dim.x / 1.5f, dim.y / 2.0f));

	gui_enum_buttons("Sort By:"_, &prof_sort);

	global_api->platform_aquire_mutex(&cache_mut);
	thread_profile* thread = dbg_cache.get(global_api->platform_this_thread_id());
	frame_profile* frame = thread->frames.back();

	gui_text(string::makef("Frame %"_, frame->number));

	profile_recurse(frame->heads);
	gui_end();

	global_api->platform_release_mutex(&cache_mut);

	POP_ALLOC();
	RESET_ARENA(&scratch);
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

void dbg_manager::register_thread(string name, u32 frames, u32 frame_size) { PROF

	global_api->platform_aquire_mutex(&cache_mut);
	
	thread_profile thread;
	thread.frame_buf_size = frames;
	thread.frame_size = frame_size;
	thread.frames = queue<frame_profile>::make(frames, alloc);
	thread.name = name;

	global_dbg->dbg_cache.insert(global_api->platform_this_thread_id(), thread);
	global_api->platform_release_mutex(&cache_mut);
}

void dbg_manager::collate() {

	PUSH_PROFILE(false) {
		global_api->platform_aquire_mutex(&cache_mut);
		thread_profile* thread = dbg_cache.get(global_api->platform_this_thread_id());

		FORQ_BEGIN(msg, this_thread_data.dbg_msgs) {

			if(msg->type == dbg_msg_type::begin_frame) {
			
				thread->num_frames++;

				if(thread->frames.full()) {
					if(!overwrite_frames) {
						break;
					}
					frame_profile rem = thread->frames.pop();
					DESTROY_POOL(&rem.pool);
				}
				frame_profile* frame = thread->frames.push(frame_profile());

				string name = string::makef("frame %"_, thread->num_frames);
				frame->pool = MAKE_POOL(name, KILOBYTES(8), alloc, false);
				frame->heads = vector<func_profile_node*>::make(2, &frame->pool);
				frame->start = msg->time;
				frame->number = thread->num_frames;
				name.destroy();
			}

			frame_profile* frame = thread->frames.back();
			if(frame) {

				PUSH_ALLOC(&frame->pool) {
					switch(msg->type) {
					case dbg_msg_type::enter_func: {

						if(!frame->current) { 	
							bool found_repeat_head = false;
							FORVEC(head, frame->heads) {
								if((*head)->context.function == msg->context.function) {
									frame->current = *head;
									found_repeat_head = true;
								}
							}
							if(!found_repeat_head) {
								frame->current = *frame->heads.push(NEW(func_profile_node));
								frame->current->children = vector<func_profile_node*>::make(4);
								frame->current->context = msg->context;
							}
							frame->current->begin = msg->time;
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
							func_profile_node* new_node = *here->children.push(NEW(func_profile_node));
							new_node->children = vector<func_profile_node*>::make(4);
							new_node->parent = here;
							here = new_node;
						}
						
						here->context = msg->context;
						here->calls++;
						here->begin = msg->time;
						frame->current = here;

					} break;

					case dbg_msg_type::exit_func: {

						timestamp runtime = msg->time - frame->current->begin;
						
						frame->current->heir += runtime;
						frame->current->begin = 0;
						frame->current = frame->current->parent;

					} break;

					case dbg_msg_type::end_frame: {
						frame->end = msg->time;

						FORVEC(it, frame->heads) {
							fixdown_self_timings(*it);
						}
					} break;
					}
				} POP_ALLOC();
			}
		} FORQ_END(msg, this_thread_data.dbg_msgs);

		global_api->platform_release_mutex(&cache_mut);
		this_thread_data.dbg_msgs.clear();

	} POP_PROFILE();
}

void dbg_manager::fixdown_self_timings(func_profile_node* node) {

	timestamp children = 0;
	FORVEC(it, node->children) {
		fixdown_self_timings(*it);
		children += (*it)->heir;
	}

	node->self = node->heir - children;
}

CALLBACK void dbg_add_log(log_message* msg, void* param) { PROF

	dbg_manager* gui = (dbg_manager*)param;

	if(gui->log_cache.len() == gui->log_cache.capacity) {

		log_message* m = gui->log_cache.front();
		DESTROY_ARENA(&m->arena);
		gui->log_cache.pop();
	}

	log_message* m = gui->log_cache.push(*msg);
	m->arena       = MAKE_ARENA("cmsg"_, msg->arena.size, gui->alloc, msg->arena.suppress_messages);
	m->call_stack  = array<code_context>::make_copy(&msg->call_stack, &m->arena);
	m->thread_name = string::make_copy(msg->thread_name, &m->arena);
	m->msg         = string::make_copy(msg->msg, &m->arena);
}

bool prof_sort_name(func_profile_node* l, func_profile_node* r) {

	return l->context.function <= r->context.function;
}

bool prof_sort_heir(func_profile_node* l, func_profile_node* r) {

	return r->heir <= l->heir;
}

bool prof_sort_self(func_profile_node* l, func_profile_node* r) {

	return r->self <= l->self;
}

bool prof_sort_calls(func_profile_node* l, func_profile_node* r) {

	return r->calls <= l->calls;
}
