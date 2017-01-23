
#pragma once

#include "..\common.hpp"
#include "..\alloc.hpp"

#include <string>
#include <iostream>
#include <vector>

#include <tbb/concurrent_hash_map.h>

enum debug_event_type {
	debug_begin_frame,
	debug_end_frame,
	debug_allocate,
	debug_deallocate,
	debug_begin_timed,
	debug_end_timed
};

struct debug_event {
	debug_event();
	debug_event(debug_event_type t);
	debug_event_type type;
};

class debug {
public:
	debug();
	~debug();

	void publish_event(debug_event evt);

private:
};

extern debug gdebug;

#ifdef _DEBUG
#define DEBUG_EVENT(e) gdebug.publish_event(e);
#else
#define DEBUG_EVENT(e)
#endif