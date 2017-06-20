
evt_manager make_evt_manager(allocator* a) {
	
	evt_manager ret;

	ret.event_queue = make_queue<platform_event>(256, a);

	return ret;
}

void start_evt_manger(evt_manager* em) {

	global_state->api->platform_set_queue_callback(&event_enqueue, &em->event_queue);
}

void destroy_evt_manager(evt_manager* em) {

	destroy_queue(&em->event_queue);
}

gui_input_state run_events(game_state* state) {

	global_state->api->platform_queue_messages(&global_state->window);
	gui_input_state ret = state->gui.input;

	while(!queue_empty(&state->evt.event_queue)) {

		platform_event evt = queue_pop(&state->evt.event_queue);

		// Exiting
		if(evt.type == event_window && evt.window.op == window_close) {
			state->running = false;
		}
		if(evt.type == event_key && evt.key.code == key_escape) {
			state->running = false;
		}

		// Window Resize
		if(evt.type == event_window && evt.window.op == window_resized) {
			global_state->window_w = evt.window.x;
			global_state->window_h = evt.window.y;
		}
		else if(evt.type == event_window && evt.window.op == window_maximized) {
			global_state->window_w = evt.window.x;
			global_state->window_h = evt.window.y;
		}

		// Debug stuff
		if(evt.type == event_key && evt.key.code == key_plus) {
			global_state->gui.style.gscale += 0.5f;
		}
		if(evt.type == event_key && evt.key.code == key_minus) {
			global_state->gui.style.gscale -= 0.5f;
		}

		// GUI: mouse
		if(evt.type == event_mouse) {
		
			if(evt.mouse.flags & mouse_flag_move) {
				ret.mousepos.x = evt.mouse.x;
				ret.mousepos.y = evt.mouse.y;
			}

			if(evt.mouse.flags & mouse_flag_wheel) {
				ret.scroll = evt.mouse.w;
			}
			
			if(evt.mouse.flags & mouse_flag_lclick) {
				ret.lclick = true;
				if(evt.mouse.flags & mouse_flag_release) {
					ret.lclick = false;
					ret.ldbl = false;
				}
				if(evt.mouse.flags & mouse_flag_double) {
					ret.lclick = false;
					ret.ldbl = true;
				}
			}

			if(evt.mouse.flags & mouse_flag_rclick) {
				ret.rclick = true;
				if(evt.mouse.flags & mouse_flag_release) {
					ret.rclick = false;
				}
			}

			if(evt.mouse.flags & mouse_flag_mclick) {
				ret.mclick = true;
				if(evt.mouse.flags & mouse_flag_release) {
					ret.mclick = false;
				}
			}
		}
	}

	return ret;
}

void event_enqueue(void* data, platform_event evt) {

	queue<platform_event>* q = (queue<platform_event>*)data;

	queue_push(q, evt);
}
