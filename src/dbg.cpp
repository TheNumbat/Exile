
dbg_manager dbg_manager::make(allocator* alloc) { PROF 

	dbg_manager ret;

	ret.dbg_cache = map<platform_thread_id,thread_profile>::make(global_api->platform_get_num_cpus(), alloc);
	ret.alloc = alloc;

	global_api->platform_create_mutex(&ret.cache_mut, false);

	return ret;
}

void dbg_manager::destroy() { PROF

	FORMAP(it, dbg_cache) {

		FORQ_BEGIN(f, it->value.frames) {
			DESTROY_ARENA(&f->arena);
		} FORQ_END(f, it->value.frames);

		it->value.frames.destroy();
	}
	global_api->platform_destroy_mutex(&cache_mut);

	dbg_cache.destroy();
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
				frame_profile frame;

				string name = string::makef(string::literal("frame %"), thread->num_frames);
				frame.arena = MAKE_ARENA(name, this_thread_data.dbg_msgs.len() * sizeof(func_profile_node), alloc, false);
				frame.heads = vector<func_profile_node*>::make(8, &frame.arena);
			
				frame_profile overwritten = thread->frames.push_overwrite(frame);
				if(overwritten.arena.memory) {
					DESTROY_ARENA(&overwritten.arena);
				}

				thread->num_frames++;
				name.destroy();
			}

#define NEW_NODE (new ((func_profile_node*)malloc(sizeof(func_profile_node))) func_profile_node)

			if(thread->frames.len() != 0) {

				frame_profile* frame = thread->frames.get((thread->num_frames - 1) % thread->frame_buf_size);
				PUSH_ALLOC(&frame->arena) {
					switch(msg->type) {
					case dbg_msg_type::enter_func: {

						if(!frame->current) {
							frame->current = *frame->heads.push(NEW_NODE);
							frame->current->children = vector<func_profile_node*>::make(4);
							frame->current->context = msg->context;
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
						frame->current = here;

					} break;

					case dbg_msg_type::exit_func: {

						frame->current = frame->current->parent;

					} break;
					}
				} POP_ALLOC();
			}
		} FORQ_END(msg, this_thread_data.dbg_msgs);

#undef NEW_NODE

		this_thread_data.dbg_msgs.clear();

	} POP_PROFILE();
}
