
#pragma once

struct engine;

typedef u32 evt_handler_id;

struct evt_handler {

	func_ptr<bool, void*, platform_event> handle;
	void* param = null;
};

struct evt_manager {

	locking_queue<platform_event> event_queue;
	map<evt_handler_id, evt_handler> handlers;
	
	evt_handler_id next_id = 1;

	static evt_manager make(allocator* a);
	void destroy();

	// NOTE(max): not thread safe	
	evt_handler_id add_handler(_FPTR* handler, void* param = null);
	void rem_handler(evt_handler_id id);

	void start();
	void run_events(engine* state);
};

void filter_dupe_window_events(queue<platform_event>* queue);

void event_enqueue(void* data, platform_event evt);

