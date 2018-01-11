
evt_manager evt_manager::make(allocator* a) { PROF
	
	evt_manager ret;

	ret.event_queue = locking_queue<platform_event>::make(256, a);

	return ret;
}

void evt_manager::start() { PROF

	global_api->platform_set_queue_callback(&event_enqueue, &event_queue);
}

void evt_manager::destroy() { PROF

	event_queue.destroy();
	global_api->platform_set_queue_callback(null, null);
}

gui_input_state run_events(game_state* state) { PROF

	global_api->platform_pump_events(&state->window);
	gui_input_state ret = state->gui.input;
	ret.scroll = 0;

	platform_event evt;
	while(state->evt.event_queue.try_pop(&evt)) {

		if(evt.type == platform_event_type::async) {
			if(evt.async.type == platform_async_type::user) {
				LOG_INFO_F("Job ID % finished!", evt.async.user_id);
			}
			if(evt.async.callback) {
				evt.async.callback();
			}
		}

		// Exit
		if(evt.type == platform_event_type::window && evt.window.op == platform_windowop::close) {
			state->running = false;
		}
		if(evt.type == platform_event_type::key && evt.key.code == platform_keycode::escape) {
			state->running = false;
		}

		// Window Resize
		if(evt.type == platform_event_type::window && evt.window.op == platform_windowop::resized) {
			state->window.w = evt.window.x;
			state->window.h = evt.window.y;
		}
		if(evt.type == platform_event_type::window && evt.window.op == platform_windowop::maximized) {
			state->window.w = evt.window.x;
			state->window.h = evt.window.y;
		}

		if(evt.type == platform_event_type::key && evt.key.code == platform_keycode::p && evt.key.flags & (u16)platform_keyflag::release) {
		}

		// GUI: mouse
		if(evt.type == platform_event_type::mouse) {
		
			if(evt.mouse.flags & (u16)platform_mouseflag::move) {
				
				i32 dx = (i32)roundf(evt.mouse.x - ret.mousepos.x);
				i32 dy = (i32)roundf(evt.mouse.y - ret.mousepos.y);
				state->world.camera.move(dx, dy, 0.5f);
				
				ret.mousepos.x = evt.mouse.x;
				ret.mousepos.y = evt.mouse.y;
			}

			if(evt.mouse.flags & (u16)platform_mouseflag::wheel) {
				ret.scroll = evt.mouse.w;
			}
			
			if(evt.mouse.flags & (u16)platform_mouseflag::lclick) {
				ret.lclick = true;
				if(evt.mouse.flags & (u16)platform_mouseflag::release) {
					ret.lclick = false;
					ret.ldbl = false;
				}
				if(evt.mouse.flags & (u16)platform_mouseflag::dbl) {
					ret.lclick = false;
					ret.ldbl = true;
				}
			}

			if(evt.mouse.flags & (u16)platform_mouseflag::rclick) {
				ret.rclick = true;
				if(evt.mouse.flags & (u16)platform_mouseflag::release) {
					ret.rclick = false;
				}
			}

			if(evt.mouse.flags & (u16)platform_mouseflag::mclick) {
				ret.mclick = true;
				if(evt.mouse.flags & (u16)platform_mouseflag::release) {
					ret.mclick = false;
				}
			}
		}
	}

	return ret;
}

void event_enqueue(void* data, platform_event evt) { PROF

	locking_queue<platform_event>* q = (locking_queue<platform_event>*)data;
	q->push(evt);
}
