
#pragma once

struct game_state;

// window/key/mouse
struct evt_handler {

	func_ptr<bool, void*, platform_event> handle;
	void* param = null;
};

struct evt_manager {

	locking_queue<platform_event> event_queue;
	vector<evt_handler> handlers;
	
	static evt_manager make(allocator* a);
	void destroy();
	
	void push_handler(_FPTR* handler, void* param = null);
	void pop_handler();

	void start();
	void run_events(game_state* state);
};

void filter_dupe_window_events(queue<platform_event>* queue);

void event_enqueue(void* data, platform_event evt);

