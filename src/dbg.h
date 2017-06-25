
#pragma once

struct cached_message {
	log_message msg;
	string fmt;
};

struct dbg_manager {

	vector<cached_message> log_cache;
	log_level lvl = log_info;

	allocator* alloc = NULL;
};

dbg_manager make_dbg_manager(allocator* alloc);
void destroy_dbg_manager(dbg_manager* dbg);

void dbg_add_log(log_message* msg, string fmt);
void render_debug_gui(game_state* state);
