
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

void filter_dupe_window_events(queue<platform_event>* queue) {

	if(queue_empty(queue)) return;

	bool found_move = false, found_resize = false;
	
	for(i32 i = (i32)queue->contents.size - 1; i >= 0; i--) {
		platform_event* evt = vector_get(&queue->contents, i);
		if(evt->type == event_window)  {
			if(evt->window.op == window_moved) {
				if(found_move) {
					vector_erase(&queue->contents, i);
				} else {
					found_move = true;
				}
			} else if(evt->window.op == window_resized) {
				if(found_resize) {
					vector_erase(&queue->contents, i);
				} else {
					found_resize = true;
				}
			}
		}
	}
}

gui_input run_events(game_state* state) {

	global_state->api->platform_queue_messages(&global_state->window);
	filter_dupe_window_events(&state->events.event_queue);

	gui_input ret = state->gui.input;
	ret.mouse.flags = 0;
	ret.mouse.w = 0;

	while(!queue_empty(&state->events.event_queue)) {

		platform_event evt = queue_pop(&state->events.event_queue);

		if(evt.type == event_window && evt.window.op == window_close) {
			state->running = false;
		}
		else if(evt.type == event_key && evt.key.code == key_escape) {
			state->running = false;
		}
		else if(evt.type == event_window && evt.window.op == window_resized) {
			LOG_DEBUG_F("window resized w: %i h: %i", evt.window.x, evt.window.y);
			global_state->window_w = evt.window.x;
			global_state->window_h = evt.window.y;
		}
		else if(evt.type == event_window && evt.window.op == window_maximized) {
			LOG_DEBUG_F("window maximized w: %i h: %i", evt.window.x, evt.window.y);
			global_state->window_w = evt.window.x;
			global_state->window_h = evt.window.y;
		}
		else if(evt.type == event_window && evt.window.op == window_moved) {
			LOG_DEBUG_F("window moved x: %i y: %i", evt.window.x, evt.window.y);
		}

		if(evt.type == event_mouse && evt.mouse.flags & mouse_flag_move) {
			ret.mouse.x = evt.mouse.x;
			ret.mouse.y = evt.mouse.y;
		} else if(evt.type == event_mouse && evt.mouse.flags & mouse_flag_wheel) {
			ret.mouse.w = evt.mouse.w;
		} 
		if(evt.type == event_mouse) {
			ret.mouse.flags |= evt.mouse.flags;
		}
	}

	return ret;
}

void event_enqueue(void* data, platform_event evt) {

	queue<platform_event>* q = (queue<platform_event>*)data;

	queue_push(q, evt);
}
