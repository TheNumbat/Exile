
#pragma once

struct evt_manager {
	locking_queue<platform_event> event_queue;

///////////////////////////////////////////////////////////////////////////////

	static evt_manager make(allocator* a);
	void destroy();
	
	void start();
};
struct game_state;

void filter_dupe_window_events(queue<platform_event>* queue);
gui_input_state run_events(game_state* state);

void event_enqueue(void* data, platform_event evt);
