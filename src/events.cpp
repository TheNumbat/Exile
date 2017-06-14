
event_manager make_event_manager(allocator* a) {
	
	event_manager ret;

	ret.event_queue = make_queue<platform_event>(256, a);

	return ret;
}

void start_event_manger(event_manager* em) {

	global_state->api->platform_set_queue_callback(&event_enqueue, &em->event_queue);
}

void destroy_event_manager(event_manager* em) {

	destroy_queue(&em->event_queue);
}

gui_input_state run_events(game_state* state) {

	global_state->api->platform_queue_messages(&global_state->window);
	gui_input_state ret = state->gui.input;

	while(!queue_empty(&state->events.event_queue)) {

		platform_event evt = queue_pop(&state->events.event_queue);

		if(evt.type == event_window && evt.window.op == window_close) {
			state->running = false;
		}
		if(evt.type == event_key && evt.key.code == key_escape) {
			state->running = false;
		}

		if(evt.type == event_window && evt.window.op == window_resized) {
			global_state->window_w = evt.window.x;
			global_state->window_h = evt.window.y;
		}
		else if(evt.type == event_window && evt.window.op == window_maximized) {
			global_state->window_w = evt.window.x;
			global_state->window_h = evt.window.y;
		}

		if(evt.type == event_mouse) {
		
			if(evt.mouse.flags & mouse_flag_move) {
				ret.mousex = evt.mouse.x;
				ret.mousey = evt.mouse.y;
			}

			if(evt.mouse.flags & mouse_flag_wheel) {
				ret.scroll = evt.mouse.w;
			}
			
			if(evt.mouse.flags & mouse_flag_lclick) {
				ret.mouse = true;
				if(evt.mouse.flags & mouse_flag_release) {
					ret.mouse = false;
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
