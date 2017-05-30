
#include "everything.h"

extern "C" game_state* start_up(platform_api* api) {

	game_state* state = (game_state*)api->platform_heap_alloc(sizeof(game_state));
	global_state = state;

	state->api = api;
	state->default_platform_allocator = MAKE_PLATFORM_ALLOCATOR("default");
	state->suppressed_platform_allocator = state->default_platform_allocator;
	state->suppressed_platform_allocator.suppress_messages = true;

	api->platform_create_mutex(&state->alloc_contexts_mutex, false);
	state->alloc_contexts = make_map<platform_thread_id,stack<allocator*>>(api->platform_get_num_cpus() + 2, &state->suppressed_platform_allocator);

	map_insert(&state->alloc_contexts, api->platform_this_thread_id(), make_stack<allocator*>(0, &state->suppressed_platform_allocator));

	state->log = make_logger(&state->suppressed_platform_allocator);

	platform_file stdout_file, log_all_file;
	api->platform_get_stdout_as_file(&stdout_file);
	api->platform_create_file(&log_all_file, string_literal("log_all.txt"), open_file_create);
	logger_add_file(&state->log, log_all_file, log_alloc);
	logger_add_file(&state->log, stdout_file, log_debug);

	LOG_INIT_THREAD(string_literal("main"));

	logger_start(&state->log);

	LOG_DEBUG("Beginning startup...");
	LOG_PUSH_CONTEXT_L("startup");

	LOG_DEBUG("Starting thread pool");
	LOG_PUSH_CONTEXT_L("threadpool");
	state->thread_pool = make_threadpool(&state->default_platform_allocator);
	threadpool_start_all(&state->thread_pool);
	LOG_POP_CONTEXT();

	LOG_DEBUG("Setting up events");
	LOG_PUSH_CONTEXT_L("events");
	state->events = make_event_manager(&state->default_platform_allocator);
	start_event_manger(&state->events);
	LOG_POP_CONTEXT();

	LOG_DEBUG("Setting up asset system");
	LOG_PUSH_CONTEXT_L("assets");
	state->test_store = make_asset_store(&state->default_platform_allocator);
	load_asset_store(&state->test_store, string_literal("cats.asset"));
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

	string version  = string_from_c_str((char*)glGetString(GL_VERSION));
	string renderer = string_from_c_str((char*)glGetString(GL_RENDERER));
	string vendor   = string_from_c_str((char*)glGetString(GL_VENDOR));

	LOG_INFO_F("GL version : %s", version.c_str);
	LOG_INFO_F("GL renderer: %s", renderer.c_str);
	LOG_INFO_F("GL vendor  : %s", vendor.c_str);

	LOG_INFO("Done with startup!");
	LOG_POP_CONTEXT();

	return state;
}

extern "C" bool main_loop(game_state* state) {

	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, state->window_w, state->window_h);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	state->api->platform_swap_buffers(&state->window);
	
	return run_events(&state->events);
}

extern "C" void shut_down(platform_api* api, game_state* state) {

	LOG_INFO("Beginning shutdown...");
	LOG_PUSH_CONTEXT_L("shutdown");

	LOG_DEBUG("Ending asset system");
	destroy_asset_store(&state->test_store);

	LOG_DEBUG("Stopping thread pool");
	threadpool_stop_all(&state->thread_pool);
	destroy_threadpool(&state->thread_pool);

	LOG_DEBUG("Destroying window");
	platform_error err = api->platform_destroy_window(&state->window);
	if(!err.good) {
		LOG_ERR_F("Failed to destroy window, error: %i", err.error);	
	}

	LOG_DEBUG("Ending events");
	destroy_event_manager(&state->events);

	LOG_DEBUG("Done with shutdown!");

	// not actually quite done but we can't log anything after this
	LOG_END_THREAD();
	logger_stop(&state->log);
	destroy_logger(&state->log);

	destroy_stack(map_get(&state->alloc_contexts, state->api->platform_this_thread_id()));
	map_erase(&state->alloc_contexts, state->api->platform_this_thread_id());
	destroy_map(&state->alloc_contexts);
	api->platform_destroy_mutex(&state->alloc_contexts_mutex);

	api->platform_heap_free(state);
}

extern "C" void on_reload(platform_api* api, game_state* state) {

	global_state = state;

	logger_start(&state->log);
	threadpool_start_all(&state->thread_pool);

	LOG_INFO("End reloading game code");
}

extern "C" void on_unload(platform_api* api, game_state* state) {
	
	LOG_INFO("Begin reloading game code");

	threadpool_stop_all(&state->thread_pool);
	logger_stop(&state->log);
}