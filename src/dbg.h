
#pragma once

struct dbg_manager {

	vector<log_message> log_cache;
	log_level lvl = log_level::info;

	allocator* alloc = null;
};

dbg_manager make_dbg_manager(log_manager* log, allocator* alloc);
void destroy_dbg_manager(dbg_manager* dbg);

void dbg_add_log(log_message* msg);
void render_debug_gui(platform_window* win, dbg_manager* dgb);
