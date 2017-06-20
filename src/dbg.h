
#pragma once

struct dbg_manager {

	vector<log_message> log_cache;
};

void dbg_add_log(log_message* msg);
void render_debug_gui(game_state* state);
