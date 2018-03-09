
dbg_manager dbg_manager::make(allocator* alloc) { PROF

	dbg_manager ret;

	ret.thread_stats = map<platform_thread_id, thread_profile*>::make(global_api->get_num_cpus(), alloc);
	ret.alloc_stats  = map<allocator*, alloc_profile*>::make(32, alloc);

	ret.log_cache = locking_queue<log_message>::make(1024, alloc);

	ret.alloc = alloc;
	ret.scratch = MAKE_ARENA("dbg scratch"_, MEGABYTES(1), alloc, false);
	ret.selected_thread = global_api->this_thread_id();
	global_api->create_mutex(&ret.stats_map_mut, false);
	global_api->create_mutex(&ret.alloc_map_mut, false);

	return ret;
}

void dbg_manager::toggle_ui() { PROF

	show_ui = !show_ui;
}

void alloc_frame_profile::destroy() { PROF

	allocs.destroy();
}

void frame_profile::destroy() { PROF
	
	DESTROY_POOL(&pool);
	FORMAP(a, allocations) {
		a->value.destroy();
	}
	allocations.destroy();
}

void thread_profile::destroy() { PROF

	FORQ_BEGIN(f, frames) {
		
		f->destroy();

	} FORQ_END(f, frames);

	frames.destroy();
	global_api->destroy_mutex(&mut);
}

void dbg_manager::destroy() { PROF

	alloc_profile totals = get_totals();
	LOG_INFO_F("% allocations remaining at debug shutdown", totals.num_allocs - totals.num_frees);

	PUSH_ALLOC(alloc);
	
	FORMAP(it, thread_stats) {
		it->value->destroy();
		free(it->value, sizeof(thread_profile));
	}
	thread_stats.destroy();
	global_api->destroy_mutex(&stats_map_mut);

	FORMAP(it, alloc_stats) {
		FORMAP(a, it->value->current_set) {
			LOG_DEBUG_F("\t% bytes in % @ %:%", a->value.size, it->key->name, string::from_c_str(a->value.last_loc.file), a->value.last_loc.line);
		}
		it->value->destroy();
		free(it->value, sizeof(alloc_profile));
	}
	alloc_stats.destroy();
	global_api->destroy_mutex(&alloc_map_mut);

	FORQ_BEGIN(it, log_cache) {
		DESTROY_ARENA(&it->arena);
	} FORQ_END(it, log_cache);
	log_cache.destroy();

	POP_ALLOC();
	DESTROY_ARENA(&scratch);
}

void dbg_manager::profile_recurse(vector<profile_node*> list) { PROF

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

		profile_node* node = *it;

		if(gui_node(string::makef("%--*|%+8|%+10|%+2"_, 35 - gui_indent_level(), string::from_c_str(node->context.function), node->heir, node->self, node->calls), &node->enabled)) {
			gui_indent();
			profile_recurse(node->children);
			gui_unindent();
		}

		gui_pop_id();
	}
}

bool operator<=(addr_info l, addr_info r) { PROF 

	return r.size <= l.size;
}

alloc_profile dbg_manager::get_totals() { PROF

	alloc_profile ret;

	global_api->aquire_mutex(&alloc_map_mut);
	FORMAP(a, alloc_stats) {
		ret.current_size += a->value->current_size;
		ret.total_allocated += a->value->total_allocated;
		ret.total_freed += a->value->total_freed;
		ret.num_allocs += a->value->num_allocs;
		ret.num_frees += a->value->num_frees;
		ret.num_reallocs += a->value->num_reallocs;
	}
	global_api->release_mutex(&alloc_map_mut);

	return ret;
}

void dbg_manager::UI() { PROF

	if(!show_ui) return;

	v2 dim = gui_window_dim();

	// NOTE(max): all the makef-ing here just comes from the scratch buffer

	gui_begin("Console"_, R2(0.0f, dim.y * 0.75f, dim.x, dim.y / 4.0f), (u16)window_flags::nowininput);

	FORQ_BEGIN(it, log_cache) {

			string level = it->fmt_level();
			gui_text(string::makef("[%-5] %"_, level, it->msg));
	
	} FORQ_END(it, log_cache);

	gui_end();

	gui_begin("Debug"_, R2(20.0f, 20.0f, dim.x / 1.5f, dim.y / 2.0f));
	
	gui_text(string::makef("FPS: %"_, 1.0f / last_frame_time));
	gui_checkbox("Pause: "_, &frame_pause);

	global_api->aquire_mutex(&alloc_map_mut);
	if(gui_node("Allocation Stats"_, &show_alloc_stats)) {

		gui_indent();
		alloc_profile totals = get_totals();
		gui_text(string::makef("Total size: %, total allocs: %, total frees: %"_, totals.current_size, totals.num_allocs, totals.num_frees));

		FORMAP(it, alloc_stats) {
			if(gui_node(string::makef("%: size: %, allocs: %, frees: %"_, it->key->name, it->value->current_size, it->value->num_allocs, it->value->num_frees), &it->value->shown)) {

				vector<addr_info> allocs = vector<addr_info>::make(it->value->current_set.size);
				FORMAP(a, it->value->current_set) {
					allocs.push(a->value);
				}
				
				allocs.stable_sort();
				
				gui_indent();
				FORVEC(a, allocs) {
					gui_text(string::makef("% bytes @ %:%"_, a->size, string::from_c_str(a->last_loc.file), a->last_loc.line));
				}
				gui_unindent();

				allocs.destroy();
			}
		}

		gui_unindent();
	}
	global_api->release_mutex(&alloc_map_mut);

	global_api->aquire_mutex(&stats_map_mut);
	map<string, platform_thread_id> threads = map<string, platform_thread_id>::make(global_api->get_num_cpus());
	FORMAP(it, thread_stats) {
		threads.insert(it->value->name, it->key);
	}
	gui_combo("Select Thread"_, threads, &selected_thread);
	threads.destroy();

	thread_profile* thread = *thread_stats.get(selected_thread);
	global_api->release_mutex(&stats_map_mut);
	
	global_api->aquire_mutex(&thread->mut);

	gui_push_id(thread->name);
	gui_int_slider("Buffer Position: "_, &thread->selected_frame, 1, thread->frame_buf_size);

	if(thread->frames.len()) {
		
		frame_profile* frame = thread->frames.get(thread->selected_frame - 1);

		f32 frame_time = (f32)(frame->perf_end - frame->perf_start) / (f32)global_api->get_perfcount_freq() * 1000.0f;
		gui_text(string::makef("Frame: %, Time: %ms"_, frame->number, frame_time));

		gui_indent();
		gui_push_id(frame->number);
		
		if(gui_node("Profile"_, &frame->show_prof)) {
			gui_indent();

			gui_enum_buttons("Sort By: "_, &prof_sort);

			profile_recurse(frame->heads);

			gui_unindent();
		}

		if(gui_node("Allocations"_, &frame->show_allocs)) {

			gui_indent();

			FORMAP(it, frame->allocations) {
				if(gui_node(it->key->name, &it->value.show)) {
					gui_indent();
					FORVEC(msg, it->value.allocs) {
						switch(msg->type) {
						case dbg_msg_type::allocate: {
							gui_text(string::makef("% bytes @ %:%"_, msg->allocate.bytes, string::from_c_str(msg->context.file), msg->context.line));
						} break;
						case dbg_msg_type::reallocate: {
							gui_text(string::makef("% bytes re@ %:%"_, msg->reallocate.to_bytes, string::from_c_str(msg->context.file), msg->context.line));
						} break;
						case dbg_msg_type::free: {
							gui_text(string::makef("free @ %:%"_, string::from_c_str(msg->context.file), msg->context.line));
						} break;
						}
					}
					gui_unindent();
				}
			}

			gui_unindent();
		}

		gui_unindent();
		gui_pop_id();
	}

	global_api->release_mutex(&thread->mut);

	gui_pop_id();
	gui_end();
}

void dbg_manager::shutdown_log(log_manager* log) { PROF
	log_out dbg_log;
	dbg_log.level = log_level::info;
	dbg_log.type = log_out_type::custom;
	dbg_log.write.set(FPTR(dbg_add_log));
	dbg_log.param = this;
	log->rem_custom_output(dbg_log);
}

void dbg_manager::setup_log(log_manager* log) { PROF
	log_out dbg_log;
	dbg_log.level = log_level::info;
	dbg_log.type = log_out_type::custom;
	dbg_log.write.set(FPTR(dbg_add_log));
	dbg_log.param = this;
	log->add_custom_output(dbg_log);
}

void dbg_manager::register_thread(u32 frames) { PROF

	PUSH_ALLOC(alloc);

	thread_profile* thread = NEW(thread_profile);
	thread->frame_buf_size = frames;
	thread->frames = queue<frame_profile>::make(frames, alloc);
	thread->name = this_thread_data.name;
	global_api->create_mutex(&thread->mut, false);

	POP_ALLOC();

	global_api->aquire_mutex(&stats_map_mut);
	global_dbg->thread_stats.insert(global_api->this_thread_id(), thread);
	global_api->release_mutex(&stats_map_mut);
}

void frame_profile::setup(string name, allocator* alloc, clock time, platform_perfcount p, u32 num) { PROF

	pool = MAKE_POOL(name, KILOBYTES(8), alloc, false);
	heads = vector<profile_node*>::make(2, &pool);
	allocations = map<allocator*, alloc_frame_profile>::make(8, alloc);
	clock_start = time;
	perf_start = p;
	number = num;
}

alloc_frame_profile alloc_frame_profile::make(allocator* alloc) { PROF

	alloc_frame_profile ret;
	
	ret.allocs = vector<dbg_msg>::make(8, alloc);

	return ret;
}

bool operator>(dbg_msg& l, dbg_msg& r) { PROF
	return l.time < r.time;
}

void dbg_manager::collate() { PROF
	
	PUSH_PROFILE(false) {
		
		collate_alloc_profile();
		collate_thread_profile();

	} POP_PROFILE();
}

void dbg_manager::collate_alloc_profile() { PROF

	FORQ_BEGIN(msg, this_thread_data.dbg_queue) {

		if(msg->type == dbg_msg_type::allocate || msg->type == dbg_msg_type::reallocate || msg->type == dbg_msg_type::free) {
			process_alloc_msg(msg);
		}

	} FORQ_END(msg, this_thread_data.dbg_queue);
}

void dbg_manager::collate_thread_profile() { PROF

	bool got_a_frame = false;
	platform_perfcount frame_perf_start = 0;

	global_api->aquire_mutex(&stats_map_mut);
	thread_profile* thread = *thread_stats.get(global_api->this_thread_id());
	global_api->release_mutex(&stats_map_mut);

	global_api->aquire_mutex(&thread->mut);

	FORQ_BEGIN(msg, this_thread_data.dbg_queue) {

		if(msg->type == dbg_msg_type::begin_frame) {
		
			frame_perf_start = msg->begin_frame.perf;
			thread->num_frames++;

			if(thread->frames.full()) {
				if(!frame_pause) {
					thread->frames.pop().destroy();
					got_a_frame = true;
				}
			} else {
				got_a_frame = true;
			}

			if(got_a_frame) {
				frame_profile* frame = thread->frames.push(frame_profile());
				string name = string::makef("frame %"_, thread->num_frames);
				frame->setup(name, alloc, msg->time, msg->begin_frame.perf, thread->num_frames);
				name.destroy();
			}
		}

		frame_profile* frame = thread->frames.back();
		if(frame && got_a_frame) {

			PUSH_ALLOC(&frame->pool) {
				switch(msg->type) {
				case dbg_msg_type::enter_func: {

					if(!frame->current) { 	
						bool found_repeat_head = false;
						FORVEC(head, frame->heads) {
							if(string::from_c_str((*head)->context.function) == string::from_c_str(msg->context.function)) {
								frame->current = *head;
								found_repeat_head = true;
							}
						}
						if(!found_repeat_head) {
							frame->current = *frame->heads.push(NEW(profile_node));
							frame->current->children = vector<profile_node*>::make(4);
							frame->current->context = msg->context;
						}
						frame->current->begin = msg->time;
						frame->current->calls++;
						break;
					}

					profile_node* here = frame->current;
					FORVEC(node, here->children) {
						if(string::from_c_str((*node)->context.function) == string::from_c_str(msg->context.function)) {
							here = *node;
							break;
						} 
					}
					if(here == frame->current) {
						profile_node* new_node = *here->children.push(NEW(profile_node));
						new_node->children = vector<profile_node*>::make(4);
						new_node->parent = here;
						here = new_node;
					}
					
					here->context = msg->context;
					here->calls++;
					here->begin = msg->time;
					frame->current = here;

				} break;

				case dbg_msg_type::exit_func: {

					clock runtime = msg->time - frame->current->begin;
					
					frame->current->heir += runtime;
					frame->current->begin = 0;
					frame->current = frame->current->parent;

				} break;

				case dbg_msg_type::end_frame: {
					frame->clock_end = msg->time;
					frame->perf_end = msg->end_frame.perf;

					FORVEC(it, frame->heads) {
						fixdown_self_timings(*it);
					}

					got_a_frame = false;
				} break;

				case dbg_msg_type::allocate: 
				case dbg_msg_type::reallocate:
				case dbg_msg_type::free: {

					process_frame_alloc_msg(frame, msg);

				} break;
				}
			} POP_ALLOC();
		}

		if(msg->type == dbg_msg_type::end_frame) {
			last_frame_time = (f32)(msg->end_frame.perf - frame_perf_start) / (f32)global_api->get_perfcount_freq();
		}

	} FORQ_END(msg, this_thread_data.dbg_queue);

	global_api->release_mutex(&thread->mut);

	this_thread_data.dbg_queue.clear();
}

alloc_profile alloc_profile::make(allocator* alloc) { PROF

	alloc_profile ret;

	ret.current_set = map<void*, addr_info>::make(64, alloc);
	global_api->create_mutex(&ret.mut, false);

	return ret;
}

alloc_profile* alloc_profile::make_new(allocator* alloc) { PROF

	PUSH_ALLOC(alloc);

	alloc_profile* ret = NEW(alloc_profile);

	ret->current_set = map<void*, addr_info>::make(64, alloc);
	global_api->create_mutex(&ret->mut, false);

	POP_ALLOC();

	return ret;
}

void alloc_profile::destroy() { PROF

	current_set.destroy();
	global_api->destroy_mutex(&mut);
}

void dbg_manager::process_frame_alloc_msg(frame_profile* frame, dbg_msg* msg) { PROF

	allocator* a = null;
	switch(msg->type) {
	case dbg_msg_type::allocate: 	a = msg->allocate.alloc; break;
	case dbg_msg_type::reallocate: 	a = msg->reallocate.alloc; break;
	case dbg_msg_type::free: 		a = msg->free.alloc; break;
	}

	alloc_frame_profile* fprofile = frame->allocations.try_get(a);

	if(!fprofile) {

		fprofile = frame->allocations.insert(a, alloc_frame_profile::make(alloc));
	}
	fprofile->allocs.push(*msg);
}

void dbg_manager::process_alloc_msg(dbg_msg* msg) { PROF

	allocator* a = null;
	switch(msg->type) {
	case dbg_msg_type::allocate: 	a = msg->allocate.alloc; break;
	case dbg_msg_type::reallocate: 	a = msg->reallocate.alloc; break;
	case dbg_msg_type::free: 		a = msg->free.alloc; break;
	}

	global_api->aquire_mutex(&alloc_map_mut);
	alloc_profile** p = alloc_stats.try_get(a);
	if(!p) {
		p = alloc_stats.insert(a, alloc_profile::make_new(alloc));
	}
	alloc_profile* profile = *p;
	global_api->release_mutex(&alloc_map_mut);

	global_api->aquire_mutex(&profile->mut);
	switch(msg->type) {
	case dbg_msg_type::allocate: {

		addr_info* info = profile->current_set.try_get(msg->allocate.to);
		if(!info) {
			info = profile->current_set.insert(msg->allocate.to, addr_info());
		}

		info->last_loc = msg->context;
		info->size += msg->allocate.bytes;

		profile->current_size += msg->allocate.bytes;
		profile->total_allocated += msg->allocate.bytes;
		profile->num_allocs++;

		if(info->size == 0) {
			profile->current_set.erase(msg->allocate.to);
		}

	} break;
	case dbg_msg_type::reallocate: {

		addr_info* from_info = profile->current_set.try_get(msg->reallocate.from);
		if(!from_info) {
			from_info = profile->current_set.insert(msg->reallocate.from, addr_info());
		}
		addr_info* to_info = profile->current_set.try_get(msg->reallocate.to);
		if(!to_info) {
			to_info = profile->current_set.insert(msg->reallocate.to, addr_info());
			from_info = profile->current_set.get(msg->reallocate.from);
		}

		from_info->last_loc = msg->context;
		from_info->size -= msg->reallocate.from_bytes;

		profile->current_size -= msg->reallocate.from_bytes;
		profile->total_freed += msg->reallocate.from_bytes;

		if(from_info->size == 0) {
			profile->current_set.erase(msg->reallocate.from);
		}

		to_info->last_loc = msg->context;
		to_info->size += msg->reallocate.to_bytes;

		profile->current_size += msg->reallocate.to_bytes;
		profile->total_allocated += msg->reallocate.to_bytes;
		profile->num_reallocs++;

		if(to_info->size == 0) {
			profile->current_set.erase(msg->reallocate.to);
		}

	} break;
	case dbg_msg_type::free: {

		addr_info* info = profile->current_set.try_get(msg->free.from);
		if(!info) {
			info = profile->current_set.insert(msg->free.from, addr_info());
		}

		info->last_loc = msg->context;
		info->size -= msg->free.bytes;

		profile->current_size -= msg->free.bytes;
		profile->total_freed += msg->free.bytes;
		profile->num_frees++;

		if(info->size == 0) {
			profile->current_set.erase(msg->free.from);
		}
		
	} break;
	}
	global_api->release_mutex(&profile->mut);
}

void dbg_manager::fixdown_self_timings(profile_node* node) { PROF

	clock children = 0;
	FORVEC(it, node->children) {
		fixdown_self_timings(*it);
		children += (*it)->heir;
	}

	node->self = node->heir - children;
}

CALLBACK void dbg_add_log(log_message* msg, void* param) { PROF

	dbg_manager* dbg = (dbg_manager*)param;

	if(dbg->log_cache.len() == dbg->log_cache.capacity) {

		log_message* m = dbg->log_cache.front();
		DESTROY_ARENA(&m->arena);
		dbg->log_cache.pop();
	}

	log_message* m = dbg->log_cache.push(*msg);
	m->arena       = MAKE_ARENA("cmsg"_, msg->arena.size, dbg->alloc, msg->arena.suppress_messages);
	m->call_stack  = array<code_context>::make_copy(&msg->call_stack, &m->arena);
	m->thread_name = string::make_copy(msg->thread_name, &m->arena);
	m->msg         = string::make_copy(msg->msg, &m->arena);
}

bool prof_sort_name(profile_node* l, profile_node* r) {

	return string::from_c_str(l->context.function) <= string::from_c_str(r->context.function);
}

bool prof_sort_heir(profile_node* l, profile_node* r) {

	return r->heir <= l->heir;
}

bool prof_sort_self(profile_node* l, profile_node* r) {

	return r->self <= l->self;
}

bool prof_sort_calls(profile_node* l, profile_node* r) {

	return r->calls <= l->calls;
}

void _prof_sec(string name, code_context context) { 
	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::enter_func;
		m.context = context;
		_memcpy_ctx(name.c_str, m.context.function, name.len);
	
		POST_MSG(m);
	}
}

void _prof_sec_end() { 
	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::exit_func;
		POST_MSG(m);
	}
}
