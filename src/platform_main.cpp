
#include <iostream>

#include "common.h"
#include "platform.h"

using std::cout;
using std::endl;

// here we treat game_state* as void* so this doesn't have to know anything about the game
typedef void* (*startup_type)(platform_api*);
typedef bool  (*main_loop_type)(void*);
typedef void  (*shutdown_type)(platform_api*, void*);

startup_type	start_up  = NULL;
main_loop_type	main_loop = NULL;
shutdown_type	shut_down = NULL;

platform_dll 				game_dll;
platform_file_attributes 	attrib;

string exe_folder;
string dll_path;
string temp_dll_path;

bool load_lib();
bool load_funcs();
bool try_reload();

int main() {

	platform_api api = platform_build_api();

	string exe_path;
	platform_error err = platform_get_bin_path(&exe_path);
	if(!err.good) {
		cout << "Error getting exe path: " << err.error << endl;
		return 1;
	}

	int idx = string_last_slash(exe_path);
	exe_folder = make_substring(exe_path, 0, idx, &platform_heap_alloc);
	dll_path = make_cat_string(exe_folder, string_literal("game.dll"), &platform_heap_alloc);
	temp_dll_path = make_cat_string(exe_folder, string_literal("game_temp.dll"), &platform_heap_alloc);

	free_string(exe_path, api.platform_heap_free);

	if(!load_lib()) {
		return 1;
	}

	err = platform_get_file_attributes(&attrib, dll_path);
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
	
	free_string(exe_folder, api.platform_heap_free);
	free_string(dll_path, api.platform_heap_free);
	free_string(temp_dll_path, api.platform_heap_free);

	return 0;
}

bool load_lib() {

	// we just spinlock here waiting for the file to unlock. This should probably be improved
	platform_error err;
	do {
		err = platform_copy_file(dll_path, temp_dll_path, true);
	} while(!err.good);

	err = platform_load_library(&game_dll, temp_dll_path);
	if(!err.good) {
		cout << "Error loading game DLL: " << err.error << endl;
		return false;
	}

	err = platform_get_file_attributes(&attrib, dll_path);
	if(!err.good) {
		cout << "Error getting DLL attributes: " << err.error << endl;
		return false;
	}

	return true;
}

bool try_reload() {

	platform_file_attributes to_test;

	platform_error err = platform_get_file_attributes(&to_test, dll_path);
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

	platform_error err = platform_get_proc_address((void**)&start_up, &game_dll, string_literal("start_up"));
	if(!err.good) {
		cout << "Error loading start_up: " << err.error << endl;
		return false;
	}

	err = platform_get_proc_address((void**)&main_loop, &game_dll, string_literal("main_loop"));
	if(!err.good) {
		cout << "Error loading main_loop: " << err.error << endl;
		return false;
	}

	err = platform_get_proc_address((void**)&shut_down, &game_dll, string_literal("shut_down"));
	if(!err.good) {
		cout << "Error loading shut_down: " << err.error << endl;
		return false;
	}

	return true;
}