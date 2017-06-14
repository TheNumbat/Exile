
#pragma once

struct event_manager {
	queue<platform_event> event_queue;
};
struct game_state;

event_manager make_event_manager(allocator* a);
void destroy_event_manager(event_manager* em);
void start_event_manger(event_manager* em);

void filter_dupe_window_events(queue<platform_event>* queue);
gui_input_state run_events(game_state* state);

void event_enqueue(void* data, platform_event evt);
