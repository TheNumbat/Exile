
#include <iostream>

#include "common.h"
#include "game.h"

#include "platform_api.h"
#include <gl/gl.h>

using std::cout;
using std::endl;

#define DLL_OUT extern "C" __declspec(dllexport)

DLL_OUT void* start_up(platform_api* api) {
	
	// TODO(max): platform-based allocation
	game_state* state = new game_state;

	state->api = api;

	platform_error err = state->api->platform_create_window(&state->window, "Window", 
													  		1280, 720);

	if(!err.good) {
		cout << "Error creating window: " << err.error << endl;
	}

	const char* version  = (const char*)glGetString(GL_VERSION);
	const char* renderer = (const char*)glGetString(GL_RENDERER);
	const char* vendor   = (const char*)glGetString(GL_VENDOR);

	cout << "Vendor  : " << vendor << endl;
	cout << "Version : " << version << endl;
	cout << "Renderer: " << renderer << endl;

	return state;
}

DLL_OUT bool main_loop(void* void_state) {

	game_state* state = (game_state*)void_state;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	state->api->platform_swap_buffers(&state->window);
	state->api->platform_wait_message();
	
	return state->api->platform_process_messages(&state->window);
}

DLL_OUT void shut_down(void* void_state) {

	game_state* state = (game_state*)void_state;

	platform_error err = state->api->platform_destroy_window(&state->window);

	if(!err.good) {
		cout << "Error destroying window: " << err.error << endl;
	}

	delete state;
}