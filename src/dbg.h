
#pragma once

struct dbg_manager {

	vector<log_message> log_cache;

	allocator* alloc = NULL;
};

dbg_manager make_dbg_manager(allocator* alloc);
void destroy_dbg_manager(dbg_manager* dbg);

void dbg_add_log(log_message* msg);
void render_debug_gui(game_state* state);
