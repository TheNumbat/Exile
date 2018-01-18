
evt_manager evt_manager::make(allocator* a) { PROF
	
	evt_manager ret;

	ret.event_queue = locking_queue<platform_event>::make(256, a);
	ret.handlers = map<evt_handler_id,evt_handler>::make(16, a);

	return ret;
}

void evt_manager::start() { PROF

	global_api->platform_set_queue_callback(&event_enqueue, &event_queue);
}

void evt_manager::destroy() { PROF

	event_queue.destroy();
	handlers.destroy();
	global_api->platform_set_queue_callback(null, null);
}

void evt_manager::run_events(engine* state) { PROF

	global_api->platform_pump_events(&state->window);

	platform_event evt;
	while(event_queue.try_pop(&evt)) {

		// Built-in event handling
		{
			// async callback
			if(evt.type == platform_event_type::async) {
				if(evt.async.type == platform_async_type::user) {
					LOG_INFO_F("Job ID % finished!", evt.async.user_id);
				}
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

void evt_manager::rem_handler(evt_handler_id id) { PROF

	handlers.erase(id);
}

void event_enqueue(void* data, platform_event evt) { PROF

	locking_queue<platform_event>* q = (locking_queue<platform_event>*)data;
	q->push(evt);
}


