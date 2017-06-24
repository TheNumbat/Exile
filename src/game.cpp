
#include <iostream>
#include "everything.h"

extern "C" game_state* start_up(platform_api* api) {

	game_state* state = (game_state*)api->platform_heap_alloc(sizeof(game_state));
	global_state = state;

	state->api = api;
	state->default_platform_allocator = make_platform_allocator(string_literal("default"), NULL);
	state->suppressed_platform_allocator = state->default_platform_allocator;
	state->suppressed_platform_allocator.suppress_messages = true;

	begin_thread(string_literal("main"), &state->suppressed_platform_allocator);

	state->log = make_logger(&state->suppressed_platform_allocator);

	platform_file stdout_file, log_all_file;
	api->platform_get_stdout_as_file(&stdout_file);
	api->platform_create_file(&log_all_file, string_literal("log_all.txt"), open_file_create);
	logger_add_file(&state->log, log_all_file, log_alloc);
	logger_add_file(&state->log, stdout_file, log_info);

	state->dbg = make_dbg_manager(&state->suppressed_platform_allocator);
	log_out dbg_log;
	dbg_log.level = log_debug;
	dbg_log.custom = true;
	dbg_log.write = &dbg_add_log;
	logger_add_output(&state->log, dbg_log);

	logger_start(&state->log);

	LOG_DEBUG("Beginning startup...");
	LOG_PUSH_CONTEXT_L("startup");

	LOG_DEBUG("Allocating transient store...")
	state->transient_arena = MAKE_ARENA("transient", MEGABYTES(16), &state->default_platform_allocator, false);

	LOG_DEBUG("Starting thread pool");
	LOG_PUSH_CONTEXT_L("threadpool");
	state->thread_pool = make_threadpool(&state->default_platform_allocator);
	threadpool_start_all(&state->thread_pool);
	LOG_POP_CONTEXT();

	LOG_DEBUG("Setting up events");
	LOG_PUSH_CONTEXT_L("events");
	state->evt = make_evt_manager(&state->default_platform_allocator);
	start_evt_manger(&state->evt);
	LOG_POP_CONTEXT();

	LOG_DEBUG("Setting up asset system");
	LOG_PUSH_CONTEXT_L("assets");
	state->default_store = make_asset_store(&state->default_platform_allocator);
	load_asset_store(&state->default_store, string_literal("assets/assets.asset"));
	LOG_POP_CONTEXT();

	LOG_DEBUG("Creating window");
	platform_error err = api->platform_create_window(&state->window, string_literal("CaveGame"), 1280, 720);
	state->window_w = 1280;
	state->window_h = 720;

	if(!err.good) {
		LOG_FATAL_F("Failed to create window, error: %i", err.error);
		api->platform_heap_free(state);
		return NULL;
	}

	LOG_DEBUG("Setting up OpenGL");
	LOG_PUSH_CONTEXT_L("ogl");
	ogl_load_global_funcs();
	state->ogl = make_opengl(&state->default_platform_allocator);
	LOG_POP_CONTEXT();

	LOG_DEBUG("Setting up GUI");
	LOG_PUSH_CONTEXT_L("gui");
	state->gui = make_gui(&state->ogl, &state->default_platform_allocator);
	gui_add_font(&state->ogl, &state->gui, string_literal("gui14"), &state->default_store);
	gui_add_font(&state->ogl, &state->gui, string_literal("gui24"), &state->default_store);
	gui_add_font(&state->ogl, &state->gui, string_literal("gui40"), &state->default_store);
	gui_add_font(&state->ogl, &state->gui, string_literal("guimono"), &state->default_store, true);
	LOG_POP_CONTEXT();

	LOG_INFO("Done with startup!");
	LOG_POP_CONTEXT();

	state->running = true;
	return state;
}

extern "C" bool main_loop(game_state* state) {

	glUseProgram(0); // why tho?? https://twitter.com/fohx/status/619887799462985729?lang=en
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	PUSH_ALLOC(&state->transient_arena) {

		gui_input_state input = run_events(state); 
		gui_begin_frame(&state->gui, input);

		render_debug_gui(state);

		gui_end_frame(&state->ogl);

	} POP_ALLOC();
	RESET_ARENA(&state->transient_arena);

	state->api->platform_swap_buffers(&state->window);

#ifdef _DEBUG
	ogl_try_reload_programs(&state->ogl);
	if(try_reload_asset_store(&state->default_store)) {
		gui_reload_fonts(&state->ogl, &state->gui);
	}
#endif

	return state->running;
}

extern "C" void shut_down(platform_api* api, game_state* state) {

	LOG_INFO("Beginning shutdown...");
	LOG_PUSH_CONTEXT_L("shutdown");

	LOG_DEBUG("Destroying GUI");
	destroy_gui(&state->gui);

	LOG_DEBUG("Destroying OpenGL")
	destroy_opengl(&state->ogl);

	LOG_DEBUG("Destroying asset system");
	destroy_asset_store(&state->default_store);

	LOG_DEBUG("Destroying thread pool");
	threadpool_stop_all(&state->thread_pool);
	destroy_threadpool(&state->thread_pool);

	LOG_DEBUG("Destroying window");
	platform_error err = api->platform_destroy_window(&state->window);
	if(!err.good) {
		LOG_ERR_F("Failed to destroy window, error: %i", err.error);	
	}

	LOG_DEBUG("Destroying events");
	destroy_evt_manager(&state->evt);

	LOG_DEBUG("Destroying transient store");
	DESTROY_ARENA(&state->transient_arena);

	LOG_DEBUG("Done with shutdown!");

	// not actually quite done but we can't log anything after this
	logger_stop(&state->log);
	destroy_logger(&state->log);

	destroy_dbg_manager(&state->dbg);

	end_thread();

	api->platform_heap_free(state);
}

extern "C" void on_reload(game_state* state) {

	global_state = state;
	ogl_load_global_funcs();

	begin_thread(string_literal("main"), &state->suppressed_platform_allocator);
	logger_start(&state->log);
	threadpool_start_all(&state->thread_pool);

	LOG_INFO("End reloading game code");
}

extern "C" void on_unload(game_state* state) {
	
	LOG_INFO("Begin reloading game code");

	threadpool_stop_all(&state->thread_pool);
	logger_stop(&state->log);

	end_thread();
}
