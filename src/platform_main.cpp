
#include <iostream>

#include "common.h"
#include "platform.h"

using std::cout;
using std::endl;

typedef void* (*startup_type)(platform_api*);
typedef bool (*main_loop_type)(void*);
typedef void (*shutdown_type)(void*);

startup_type start_up = NULL;
main_loop_type main_loop = NULL;
shutdown_type shut_down = NULL;

int main() {

	platform_api api = platform_build_api();
	platform_dll game_dll;

	platform_error err = platform_load_library(&game_dll, "game.dll");

	if(!err.good) {
		cout << "Error loading game DLL: " << err.error << endl;
	}

	err = platform_get_proc_address((void**)&start_up, &game_dll, "start_up");
	if(!err.good) {
		cout << "Error loading start_up: " << err.error << endl;
	}

	err = platform_get_proc_address((void**)&main_loop, &game_dll, "main_loop");
	if(!err.good) {
		cout << "Error loading main_loop: " << err.error << endl;
	}

	err = platform_get_proc_address((void**)&shut_down, &game_dll, "shut_down");
	if(!err.good) {
		cout << "Error loading shut_down: " << err.error << endl;
	}

	void* game_state = (*start_up)(&api);

	while((*main_loop)(game_state)) {

	}

	(*shut_down)(game_state);

	platform_free_library(&game_dll);

	return 0;
}

