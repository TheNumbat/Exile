
#include <iostream>

#include "common.h"
#include "platform.h"

using std::cout;
using std::endl;

// here we treat game_state* as void* so this doesn't have to know anything about the game
typedef void* (*startup_type)(platform_api*);
typedef bool  (*main_loop_type)(void*);
typedef void  (*shutdown_type)(platform_api*, void*);

startup_type	start_up = NULL;
main_loop_type	main_loop = NULL;
shutdown_type	shut_down = NULL;

platform_dll 				game_dll;
platform_file_attributes 	attrib;

bool load_lib();
bool load_funcs();
bool try_reload();

int main() {

	platform_api api = platform_build_api();

	if(!load_lib()) {
		return 1;
	}

	platform_error err = platform_get_file_attributes(&attrib, "../build/game.dll");
	if(!err.good) {
		cout << "Error getting DLL attributes: " << err.error << endl;
		return 1;
	}

	if(!load_funcs()) {
		return 1;
	}

	void* game_state = (*start_up)(&api);

	while((*main_loop)(game_state)) {
		if(!try_reload()) {
			return 1;
		}
	}

	(*shut_down)(&api, game_state);

	platform_free_library(&game_dll);

	return 0;
}

bool load_lib() {

	// we just spinlock here waiting for the file to unlock. This should probably be improved
	platform_error err;
	do {
		err = platform_copy_file("../build/game.dll", "../build/game_temp.dll", true);
	} while(!err.good);

	err = platform_load_library(&game_dll, "../build/game_temp.dll");
	if(!err.good) {
		cout << "Error loading game DLL: " << err.error << endl;
		return false;
	}

	err = platform_get_file_attributes(&attrib, "../build/game.dll");
	if(!err.good) {
		cout << "Error getting DLL attributes: " << err.error << endl;
		return false;
	}

	return true;
}

bool try_reload() {

	platform_file_attributes to_test;

	platform_error err = platform_get_file_attributes(&to_test, "../build/game.dll");
	if(!err.good) {
		cout << "Error getting DLL attributes: " << err.error << endl;
		return false;
	}

	if(platform_test_file_written(&attrib, &to_test)) {
		
		cout << "Reloading Game Code" << endl;

		attrib = to_test;

		platform_free_library(&game_dll);

		if(!load_lib()) return false;

		if(!load_funcs()) return false;
	}

	return true;
}

bool load_funcs() {

	platform_error err = platform_get_proc_address((void**)&start_up, &game_dll, "start_up");
	if(!err.good) {
		cout << "Error loading start_up: " << err.error << endl;
		return false;
	}

	err = platform_get_proc_address((void**)&main_loop, &game_dll, "main_loop");
	if(!err.good) {
		cout << "Error loading main_loop: " << err.error << endl;
		return false;
	}

	err = platform_get_proc_address((void**)&shut_down, &game_dll, "shut_down");
	if(!err.good) {
		cout << "Error loading shut_down: " << err.error << endl;
		return false;
	}

	return true;
}