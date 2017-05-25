
#include "common.h"
#include "game.h"

#include <gl/gl.h>

extern "C" game_state* start_up(platform_api* api) {

	game_state* state = (game_state*)api->platform_heap_alloc(sizeof(game_state));

	global_platform_api = api;

	state->api = api;
	state->default_platform_allocator = MAKE_PLATFORM_ALLOCATOR();
	state->thread_pool = make_threadpool(&state->default_platform_allocator);

	platform_error err = api->platform_create_mutex(&state->alloc_contexts_mutex, true);
	if(!err.good) {

		api->platform_heap_free(state);
		return NULL;
	}

	state->global_alloc_contexts = make_map<platform_thread_id,stack<allocator*>>(api->platform_get_num_cpus(), &state->default_platform_allocator);
	global_alloc_contexts = &state->global_alloc_contexts;
	map_insert(global_alloc_contexts, api->platform_this_thread_id(), make_stack<allocator*>(0, &state->default_platform_allocator));

	api->platform_release_mutex(&state->alloc_contexts_mutex);

	global_alloc_contexts_mutex = &state->alloc_contexts_mutex;

	threadpool_start_all(&state->thread_pool);

	err = api->platform_create_window(&state->window, string_literal("Window"), 
						  			  1280, 720);

	if(!err.good) {

		api->platform_heap_free(state);
		return NULL;
	}

	string version  = string_from_c_str((char*)glGetString(GL_VERSION));
	string renderer = string_from_c_str((char*)glGetString(GL_RENDERER));
	string vendor   = string_from_c_str((char*)glGetString(GL_VENDOR));

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

	api->platform_aquire_mutex(&state->alloc_contexts_mutex, -1);
	destroy_stack(&map_get(global_alloc_contexts, global_platform_api->platform_this_thread_id()));
	map_erase(global_alloc_contexts, global_platform_api->platform_this_thread_id());
	destroy_map(&state->global_alloc_contexts);
	api->platform_release_mutex(&state->alloc_contexts_mutex);
	api->platform_destroy_mutex(&state->alloc_contexts_mutex);

	platform_error err = api->platform_destroy_window(&state->window);

	if(!err.good) {
		
	}

	api->platform_heap_free(state);
}

extern "C" void on_reload(platform_api* api, game_state* state) {

	global_alloc_contexts 		= &state->global_alloc_contexts;
	global_platform_api			= state->api;
	global_alloc_contexts_mutex = &state->alloc_contexts_mutex;

	threadpool_start_all(&state->thread_pool);
}

extern "C" void on_unload(platform_api* api, game_state* state) {
	
	threadpool_stop_all(&state->thread_pool);
}