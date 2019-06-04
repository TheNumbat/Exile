
#pragma once

struct engine {

	log_manager log;
	ogl_manager ogl;
	imgui_manager imgui;
	dbg_manager dbg;
	evt_manager evt;

	bool running = false;
	platform_window window;
	platform_api* platform = null;

// private:

	platform_allocator basic_a;
	func_ptr_state func_state;

	platform_allocator log_a, ogl_a, imgui_a, dbg_a, evt_a; // idk about this
	
	void* game_state = null;
	bool apply_window_settings = false;
};

void* start_up_game(engine* e);
void run_game(void* game);
void shut_down_game(void* game);
void gl_reload_game(void* game);
void reload_game(engine* e, void* game);
void unload_game(engine* e, void* game);
