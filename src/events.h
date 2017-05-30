
#pragma once

struct event_manager {
	queue<platform_event> event_queue;
};

event_manager make_event_manager(allocator* a);
void destroy_event_manager(event_manager* em);
void start_event_manger(event_manager* em);

void filter_dupe_window_events(queue<platform_event>* queue);
bool run_events(event_manager* em);

void event_enqueue(void* data, platform_event evt);
