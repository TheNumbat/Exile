
#include "everything.h"

#include <iostream>
using std::cout;
using std::endl;

extern "C" game_state* start_up(platform_api* api) {

	game_state* state = (game_state*)api->platform_heap_alloc(sizeof(game_state));
	global_state = state;

	state->api = api;
	state->default_platform_allocator = MAKE_PLATFORM_ALLOCATOR();

	api->platform_create_mutex(&state->alloc_contexts_mutex, false);
	state->alloc_contexts = make_map<platform_thread_id,stack<allocator*>>(api->platform_get_num_cpus() + 2, &state->default_platform_allocator);

	map_insert(&state->alloc_contexts, api->platform_this_thread_id(), make_stack<allocator*>(0, &state->default_platform_allocator));

	state->log = make_logger(&state->default_platform_allocator);

	platform_file stdout_file;
	api->platform_get_stdout_as_file(&stdout_file);
	logger_add_file(&state->log, stdout_file, log_info);
	LOG_INIT_THREAD(string_literal("main"));

	logger_start(&state->log);

	state->thread_pool = make_threadpool(&state->default_platform_allocator);
	threadpool_start_all(&state->thread_pool);

	platform_error err = api->platform_create_window(&state->window, string_literal("Window"), 
						  			  1280, 720);

	if(!err.good) {

		api->platform_heap_free(state);
		return NULL;
	}

	string version  = string_from_c_str((char*)glGetString(GL_VERSION));
	string renderer = string_from_c_str((char*)glGetString(GL_RENDERER));
	string vendor   = string_from_c_str((char*)glGetString(GL_VENDOR));

#if 0 // basic map vs unordered_map performance testing
	map<i32,i32> test = make_map<i32,i32>(MAP_CAPACITY(1000000), &state->default_platform_allocator);
	for(i32 i = 0; i < 1000000; i++)
		map_insert(&test, i, i);		
	for(i32 i = 0; i < 1000000; i++)	// 212
		map_erase(&test, i);
	destroy_map(&test);					// 434

	{
		std::unordered_map<i32, i32> test2;
		test2.reserve(1000000);
		for(i32 i = 0; i < 1000000; i++)
			test2.insert({i, i});
		for(i32 i = 0; i < 1000000; i++)
			test2.erase(i);
	}
#endif
	return state;
}

extern "C" bool main_loop(game_state* state) {

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	state->api->platform_swap_buffers(&state->window);
	
	return state->api->platform_process_messages(&state->window);
}

extern "C" void shut_down(platform_api* api, game_state* state) {

	threadpool_stop_all(&state->thread_pool);
	destroy_threadpool(&state->thread_pool);

	LOG_END_THREAD();
	logger_stop(&state->log);
	destroy_logger(&state->log);

	destroy_stack(map_get(&state->alloc_contexts, state->api->platform_this_thread_id()));
	map_erase(&state->alloc_contexts, state->api->platform_this_thread_id());
	destroy_map(&state->alloc_contexts);
	api->platform_destroy_mutex(&state->alloc_contexts_mutex);

	platform_error err = api->platform_destroy_window(&state->window);

	if(!err.good) {
		LOG_ERR_F("Failed to destroy window, error: %i", err.error);	
	}

	api->platform_heap_free(state);
}

extern "C" void on_reload(platform_api* api, game_state* state) {

	global_state = state;

	logger_start(&state->log);
	threadpool_start_all(&state->thread_pool);
}

extern "C" void on_unload(platform_api* api, game_state* state) {
	
	threadpool_stop_all(&state->thread_pool);
	logger_stop(&state->log);
}