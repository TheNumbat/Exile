
#pragma once

struct dbg_gui {
	guiid info = 0;
	guiid window = 0; bool bwindow = false;
		guiid size = 0;
};

void render_debug_gui(game_state* state);
