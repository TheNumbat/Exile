
#pragma once

struct evt_manager {
	con_queue<platform_event> event_queue;
};
struct game_state;

evt_manager make_evt_manager(allocator* a);
void destroy_evt_manager(evt_manager* em);
void start_evt_manger(evt_manager* em);

void filter_dupe_window_events(queue<platform_event>* queue);
gui_input_state run_events(game_state* state);

void event_enqueue(void* data, platform_event evt);
