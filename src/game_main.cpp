
#include <iostream>
using std::cout;
using std::endl;

#include "common.h"
#include "game.h"

#include <gl/gl.h>

extern "C" game_state* start_up(platform_api* api) {
	
	game_state* state = (game_state*)api->platform_heap_alloc(sizeof(game_state));

	state->api = api;
	state->default_platform_allocator = MAKE_PLATFORM_ALLOCATOR(api);
	state->global_alloc_context_stack = make_stack<allocator*>(0, &state->default_platform_allocator);

	platform_error err = state->api->platform_create_window(&state->window, string_literal("Window"), 
													  		1280, 720);

	if(!err.good) {
		cout << "Error creating window: " << err.error << endl;
		api->platform_heap_free(state);
		return NULL;
	}

	string version  = string_from_c_str((char*)glGetString(GL_VERSION));
	string renderer = string_from_c_str((char*)glGetString(GL_RENDERER));
	string vendor   = string_from_c_str((char*)glGetString(GL_VENDOR));

	cout << "Vendor  : " << vendor.c_str   << endl;
	cout << "Version : " << version.c_str  << endl;
	cout << "Renderer: " << renderer.c_str << endl;

	return state;
}

extern "C" bool main_loop(game_state* state) {

	arena_allocator arena = MAKE_ARENA_ALLOCATOR(MEGABYTES(0), &state->default_platform_allocator);
	PUSH_ALLOC(&arena) {

		//void* test = malloc(1024);
		//assert(test)
		//string test = make_string(1024);
		//assert(test.c_str);
		array<int> a = make_array<int>(1024);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		state->api->platform_swap_buffers(&state->window);

		//free(test);
		//free_string(test);
		destroy_array(&a);

	} POP_ALLOC();

	return state->api->platform_process_messages(&state->window);
}

extern "C" void shut_down(platform_api* api, game_state* state) {

	destroy_stack(&state->global_alloc_context_stack);

	platform_error err = api->platform_destroy_window(&state->window);

	if(!err.good) {
		cout << "Error destroying window: " << err.error << endl;
	}

	api->platform_heap_free(state);
}

extern "C" void on_load(platform_api* api, game_state* state) {

	global_alloc_context_stack = &state->global_alloc_context_stack;

}

extern "C" void on_unload(platform_api* api, game_state* state) {
	
}