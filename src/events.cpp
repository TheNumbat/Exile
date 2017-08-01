
evt_manager make_evt_manager(allocator* a) { PROF
	
	evt_manager ret;

	ret.event_queue = make_con_queue<platform_event>(256, a);

	return ret;
}

void start_evt_manger(evt_manager* em) { PROF

	global_state->api->platform_set_queue_callback(&event_enqueue, &em->event_queue);
}

void destroy_evt_manager(evt_manager* em) { PROF

	destroy_con_queue(&em->event_queue);
}

gui_input_state run_events(game_state* state) { PROF

	global_state->api->platform_pump_events(&global_state->window);
	gui_input_state ret = state->gui.input;
	ret.scroll = 0;

	platform_event evt;
	while(queue_try_pop(&state->evt.event_queue, &evt)) {

		if(evt.type == platform_event_type::async) {
			if(evt.async.type == platform_async_type::user) {
				LOG_INFO_F("Job ID % finished!", evt.async.user_id);
			}
			if(evt.async.callback) {
				evt.async.callback();
			}
		}

		// Exiting
		if(evt.type == platform_event_type::window && evt.window.op == platform_windowop::close) {
			state->running = false;
		}
		if(evt.type == platform_event_type::key && evt.key.code == platform_keycode::escape) {
			state->running = false;
		}

		// Window Resize
		if(evt.type == platform_event_type::window && evt.window.op == platform_windowop::resized) {
			global_state->window_w = evt.window.x;
			global_state->window_h = evt.window.y;
		}
		else if(evt.type == platform_event_type::window && evt.window.op == platform_windowop::maximized) {
			global_state->window_w = evt.window.x;
			global_state->window_h = evt.window.y;
		}

		// Debug stuff
		if(evt.type == platform_event_type::key && evt.key.flags & (u16)platform_keyflag::release && evt.key.code == platform_keycode::plus) {
			global_state->gui.style.gscale *= 1.5f;
		}
		if(evt.type == platform_event_type::key && evt.key.flags & (u16)platform_keyflag::release && evt.key.code == platform_keycode::minus) {
			global_state->gui.style.gscale /= 1.5f;
		}

		// GUI: mouse
		if(evt.type == platform_event_type::mouse) {
		
			if(evt.mouse.flags & (u16)platform_mouseflag::move) {
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

	// TODO(max) IMPORTANT(max): this needs to be thread-safe now
	con_queue<platform_event>* q = (con_queue<platform_event>*)data;
	queue_push(q, evt);
}
