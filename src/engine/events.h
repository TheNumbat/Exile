
#pragma once

struct engine;

typedef u32 evt_handler_id;
typedef u32 evt_state_id;

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
	evt_handler_id add_handler(evt_handler handler);
	void rem_handler(evt_handler_id id);

	void start();
	void run_events(engine* state);
};

struct evt_id_transition {
	evt_state_id from;
	evt_state_id to;
};

struct evt_transition_callback {
	func_ptr<void, void*> func;
	void* param = null;
};

bool operator==(evt_id_transition l, evt_id_transition r);
u32 hash(evt_id_transition trans);

struct evt_state_machine {

	evt_state_id next_id = 1;

	map<evt_state_id, evt_handler> states;
	map<evt_id_transition, evt_transition_callback> transitions;

	evt_state_id active_state = 0;
	evt_handler_id active_id  = 0;

	evt_manager* mgr = null;

	static evt_state_machine make(evt_manager* mgr, allocator* a);
	void destroy();

	evt_state_id add_state(_FPTR* handler, void* param = null);
	void set_state(evt_state_id id);
	void rem_state(evt_state_id id);

	void transition(evt_state_id to);
	void add_transition(evt_state_id from, evt_state_id to, _FPTR* func, void* param);
};

void event_enqueue(void* data, platform_event evt);
