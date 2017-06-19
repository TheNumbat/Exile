
// platform layer setup + game code loading/reloading

#pragma warning(disable : 4007)

#include "basic_types.h"

#include "str/platform_strings.h"
#include "str/platform_strings.cpp"

#include "platform_api.h"
#include "platform.h"
#include "platform_win32.cpp"

// here we treat game_state* as void* so this doesn't have to know anything about the game
typedef void* (*startup_type)(platform_api*);
typedef bool  (*main_loop_type)(void*);
typedef void  (*shut_down_type)(platform_api*, void*);
typedef main_loop_type on_reload_type;
typedef main_loop_type on_unload_type;

startup_type	start_up  = NULL;
main_loop_type	main_loop = NULL;
shut_down_type	shut_down = NULL;
on_reload_type 	on_reload = NULL;
on_unload_type  on_unload = NULL;

platform_dll 				game_dll;
platform_file_attributes 	attrib;
platform_api 				api;
void* 						game_state;

string exe_folder;
string dll_path;
string temp_dll_path;

bool load_lib();
bool load_funcs();
bool try_reload();

int main() {

	api = platform_build_api();

	string exe_path;
	platform_error err = platform_get_bin_path(&exe_path);
	if(!err.good) {
		return 1;
	}

	int idx = string_last_slash(exe_path);
	exe_folder 	  = make_substring(exe_path, 0, idx, &platform_heap_alloc);
	dll_path 	  = make_cat_string(exe_folder, string_literal("game.dll"), &platform_heap_alloc);
	temp_dll_path = make_cat_string(exe_folder, string_literal("game_temp.dll"), &platform_heap_alloc);

	free_string(exe_path, api.platform_heap_free);

	if(!load_lib()) {
		return 1;
	}

	err = platform_get_file_attributes(&attrib, dll_path);
	if(!err.good) {
		return 1;
	}

	if(!load_funcs()) {
		return 1;
	}

	game_state = (*start_up)(&api);

	if(game_state == NULL) {
		return 1;
	}

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

	// we just spinlock here waiting for the file to unlock
	// TODO(max): cap infinite loop
	platform_error err;
	u32 itr = 0;
	do {
		itr++;
		err = platform_copy_file(dll_path, temp_dll_path, true);
	} while(err.error == PLATFORM_SHARING_ERROR && itr < 100000);

	err = platform_load_library(&game_dll, temp_dll_path);
	if(!err.good) {
		return false;
	}

	err = platform_get_file_attributes(&attrib, dll_path);
	if(!err.good) {
		return false;
	}

	return true;
}

bool try_reload() {

	platform_file_attributes to_test;

	platform_error err = platform_get_file_attributes(&to_test, dll_path);
	if(!err.good) {
		return false;
	}

	if(platform_test_file_written(&attrib, &to_test)) {
		
		attrib = to_test;

		(*on_unload)(game_state);

		platform_free_library(&game_dll);

		if(!load_lib()) return false;

		if(!load_funcs()) return false;

		(*on_reload)(game_state);
	}

	return true;
}

bool load_funcs() {

	platform_error err = platform_get_proc_address((void**)&start_up, &game_dll, string_literal("start_up"));
	if(!err.good) {
		return false;
	}

	err = platform_get_proc_address((void**)&main_loop, &game_dll, string_literal("main_loop"));
	if(!err.good) {
		return false;
	}

	err = platform_get_proc_address((void**)&shut_down, &game_dll, string_literal("shut_down"));
	if(!err.good) {
		return false;
	}

	err = platform_get_proc_address((void**)&on_reload, &game_dll, string_literal("on_reload"));
	if(!err.good) {
		return false;
	}

	err = platform_get_proc_address((void**)&on_unload, &game_dll, string_literal("on_unload"));
	if(!err.good) {
		return false;
	}

	return true;
}