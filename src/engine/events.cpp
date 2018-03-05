
evt_manager evt_manager::make(allocator* a) { PROF
	
	evt_manager ret;

	ret.event_queue = locking_queue<platform_event>::make(256, a);
	ret.handlers = map<evt_handler_id,evt_handler>::make(16, a);

	return ret;
}

void evt_manager::start() { PROF

	global_api->set_queue_callback(&event_enqueue, &event_queue);
}

void evt_manager::destroy() { PROF

	event_queue.destroy();
	handlers.destroy();
	global_api->set_queue_callback(null, null);
}

void evt_manager::run_events(engine* state) { PROF

	global_api->pump_events(&state->window);

	platform_event evt;
	while(event_queue.try_pop(&evt)) {

		// Built-in event handling
		{
			// async callback
			if(evt.type == platform_event_type::async) {
				if(evt.async.callback) {
					evt.async.callback();
				}
			}

			// Exit
			else if(evt.type == platform_event_type::window && evt.window.op == platform_windowop::close) {
				state->running = false;
			}

			// Window Resize
			else if(evt.type == platform_event_type::window && evt.window.op == platform_windowop::resized) {
				state->window.w = evt.window.x;
				state->window.h = evt.window.y;
			}
			else if(evt.type == platform_event_type::window && evt.window.op == platform_windowop::maximized) {
				state->window.w = evt.window.x;
				state->window.h = evt.window.y;
			}
		}

		FORMAP(it, handlers) {

			if(it->value.handle(it->value.param, evt)) {
				break;
			}
		}
	}
}

evt_handler_id evt_manager::add_handler(_FPTR* handler, void* param) { PROF

	evt_handler h;
	h.handle.set(handler);
	h.param = param;
	
	handlers.insert(next_id++, h);

	return next_id - 1;
}

evt_handler_id evt_manager::add_handler(evt_handler handler) { PROF

	handlers.insert(next_id++, handler);

	return next_id - 1;
}

void evt_manager::rem_handler(evt_handler_id id) { PROF

	handlers.erase(id);
}

void event_enqueue(void* data, platform_event evt) { PROF

	locking_queue<platform_event>* q = (locking_queue<platform_event>*)data;
	q->push(evt);
}

evt_state_machine evt_state_machine::make(evt_manager* mgr, allocator* a) { PROF

	evt_state_machine ret;

	ret.states = map<evt_state_id, evt_handler>::make(16, a);
	ret.transitions = map<evt_id_transition, evt_transition_callback>::make(256, a);
	ret.mgr = mgr;

	return ret;
}

void evt_state_machine::destroy() { PROF

	if(active_id) {
		mgr->rem_handler(active_id);
		active_id = active_state = 0;
	}

	states.destroy();
	transitions.destroy();
}

evt_state_id evt_state_machine::add_state(_FPTR* handler, void* param) { PROF

	evt_handler h;
	h.handle.set(handler);
	h.param = param;
	
	states.insert(next_id++, h);

	return next_id - 1;
}

void evt_state_machine::rem_state(evt_state_id id) { PROF

	if(active_state == id) {
		mgr->rem_handler(active_id);
		active_id = 0;
		active_state = 0;
	}

	states.erase(id);

	FORMAP(it, transitions) {
		if(it->key.from == id || it->key.to == id) {
			transitions.erase(it->key);
		}
	}
}

void evt_state_machine::set_state(evt_state_id id) { PROF

	if(active_id) {
		mgr->rem_handler(active_id);
		active_id = 0;
	}

	evt_handler* h = states.get(id);
	LOG_DEBUG_ASSERT(h);

	active_state = id;
	active_id = mgr->add_handler(*h);
}

void evt_state_machine::transition(evt_state_id to) { PROF

	evt_id_transition trans;
	trans.from = active_state;
	trans.to = to;

	evt_transition_callback* callback = transitions.try_get(trans);

	if(callback) {
		callback->func(callback->param);
	}

	set_state(to);
}

void evt_state_machine::add_transition(evt_state_id from, evt_state_id to, _FPTR* func, void* param) { PROF

	evt_id_transition trans = {from, to};
	evt_transition_callback callback;
	callback.func.set(func);
	callback.param = param;

	LOG_DEBUG_ASSERT(!transitions.try_get(trans));

	transitions.insert(trans, callback);
}

u32 hash(evt_id_transition trans) { PROF
	return hash(trans.from) ^ hash(trans.to);
}

bool operator==(evt_id_transition l, evt_id_transition r) { PROF
	return l.from == r.from && l.to == r.to;
}
