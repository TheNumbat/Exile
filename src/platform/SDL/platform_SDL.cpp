
#include "platform_SDL.h"

#include <SDL2/SDL.h>

platform_api platform_build_api() {

	platform_api ret;

	ret.platform_get_perfcount			= &sdl_get_perfcount;
	ret.platform_get_perfcount_freq		= &sdl_get_perfcount_freq;
	ret.platform_create_window			= &sdl_create_window;
	ret.platform_destroy_window			= &sdl_destroy_window;
	ret.platform_swap_buffers			= &sdl_swap_buffers;
	ret.platform_set_queue_callback		= &sdl_set_queue_callback;
	ret.platform_pump_events			= &sdl_pump_events;
	ret.platform_queue_event 			= &sdl_queue_event;
	ret.platform_wait_message			= &sdl_wait_message;
	ret.platform_load_library			= &sdl_load_library;
	ret.platform_free_library			= &sdl_free_library;
	ret.platform_get_proc_address 		= &sdl_get_proc_address;
	ret.platform_get_file_attributes 	= &sdl_get_file_attributes;
	ret.platform_test_file_written		= &sdl_test_file_written;
	ret.platform_copy_file				= &sdl_copy_file;
	ret.platform_heap_alloc				= &sdl_heap_alloc;
	ret.platform_heap_realloc			= &sdl_heap_realloc;
	ret.platform_heap_free				= &sdl_heap_free;
	ret.platform_get_bin_path			= &sdl_get_bin_path;
	ret.platform_create_thread			= &sdl_create_thread;
	ret.platform_this_thread_id			= &sdl_this_thread_id;
	ret.platform_terminate_thread		= &sdl_terminate_thread;
	ret.platform_exit_this_thread		= &sdl_exit_this_thread;
	ret.platform_thread_sleep			= &sdl_thread_sleep;
	ret.platform_create_semaphore		= &sdl_create_semaphore;
	ret.platform_destroy_semaphore		= &sdl_destroy_semaphore;
	ret.platform_signal_semaphore		= &sdl_signal_semaphore;
	ret.platform_wait_semaphore			= &sdl_wait_semaphore;
	ret.platform_create_mutex			= &sdl_create_mutex;
	ret.platform_destroy_mutex			= &sdl_destroy_mutex;
	ret.platform_aquire_mutex			= &sdl_aquire_mutex;
	ret.platform_try_aquire_mutex		= &sdl_try_aquire_mutex;
	ret.platform_release_mutex			= &sdl_release_mutex;
	ret.platform_destroy_thread			= &sdl_destroy_thread;
	ret.platform_get_num_cpus			= &sdl_get_num_cpus;
	ret.platform_join_thread			= &sdl_join_thread;
	ret.platform_create_file			= &sdl_create_file;
	ret.platform_close_file				= &sdl_close_file;
	ret.platform_write_file				= &sdl_write_file;
	ret.platform_read_file				= &sdl_read_file;
	ret.platform_get_stdout_as_file		= &sdl_get_stdout_as_file;
	ret.platform_get_timef				= &sdl_get_timef;
	ret.platform_make_timef				= &sdl_make_timef;
	ret.platform_get_window_size		= &sdl_get_window_size;
	ret.platform_write_stdout			= &sdl_write_stdout;
	ret.platform_file_size				= &sdl_file_size;
	ret.platform_get_glproc				= &sdl_get_glproc;
	ret.platform_keydown				= &sdl_keydown;
	ret.platform_is_debugging			= &sdl_is_debugging;
	ret.platform_debug_break			= &sdl_debug_break;
	ret.platform_set_cursor				= &sdl_set_cursor;
	ret.platform_this_dll				= &sdl_this_dll;
	ret.platform_capture_mouse			= &sdl_capture_mouse;
	ret.platform_release_mouse			= &sdl_release_mouse;
	ret.platform_set_cursor_pos			= &sdl_set_cursor_pos;
	ret.platform_window_focused 		= &sdl_window_focused;

	return ret;
}

bool sdl_window_focused(platform_window* win) {

	return false;
}

platform_error sdl_set_cursor_pos(platform_window* win, i32 x, i32 y) {
	
}

void sdl_capture_mouse(platform_window* win) {

}

void sdl_release_mouse() {

}

platform_perfcount sdl_get_perfcount() {

	return 0;
}

platform_perfcount sdl_get_perfcount_freq() {

	return 0;
}

platform_error sdl_this_dll(platform_dll* dll) {

	platform_error ret;

	return ret;
}

void sdl_debug_break() {

}

bool sdl_is_debugging() {
	
	return false;
}

void sdl_set_cursor(cursors c) {

}

platform_error sdl_create_window(platform_window* window, string title, u32 width, u32 height) {

	platform_error ret;
	u32  flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZEABLE | SDL_WINDOW_BORDERLESS;
	window.window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (!window.window)
	{
		ret.good = false;
		ret.error_message = string::from_c_str(SDL_GetError());
	}
	return ret;
}

platform_error sdl_destroy_window(platform_window* window) {

	platform_error ret;

	return ret;
}

platform_error sdl_get_window_size(platform_window* window, i32* w, i32* h) {

	platform_error ret;

	return ret;
}

platform_error sdl_swap_buffers(platform_window* window) {

	platform_error ret;

	return ret;
}

void sdl_set_queue_callback(void (*enqueue)(void* queue_param, platform_event evt), void* queue_param) {

}

void sdl_pump_events(platform_window* window) {

}

void sdl_queue_event(platform_event evt) {

}

platform_error sdl_wait_message() {

	platform_error ret;

	return ret;
}

bool sdl_keydown(platform_keycode key) {

	return false;
}

platform_error sdl_load_library(platform_dll* dll, string file_path) {

	platform_error ret;

	return ret;
}

platform_error sdl_free_library(platform_dll* dll) {

	platform_error ret;

	return ret;
}

platform_error sdl_get_proc_address(void** address, platform_dll* dll, string name) {

	platform_error ret;

	return ret;
}

void* sdl_get_glproc(string name) {

	return null;
}

platform_error sdl_copy_file(string source, string dest, bool overwrite) {

	platform_error ret;

	return ret;
}

platform_error sdl_get_file_attributes(platform_file_attributes* attrib, string file_path) {

	platform_error ret;

	return ret;
}

bool sdl_test_file_written(platform_file_attributes* first, platform_file_attributes* second) {

	return false;
}

platform_error sdl_create_file(platform_file* file, string path, platform_file_open_op mode) {

	platform_error ret;

	return ret;
}

platform_error sdl_close_file(platform_file* file) {

	platform_error ret;

	return ret;
}

platform_error sdl_write_file(platform_file* file, void* mem, u32 bytes) {

	platform_error ret;

	return ret;
}

platform_error sdl_read_file(platform_file* file, void* mem, u32 bytes) {

	platform_error ret;

	return ret;
}

u32	sdl_file_size(platform_file* file) {

	return 0;
}

platform_error sdl_get_stdout_as_file(platform_file* file) {

	platform_error ret;

	return ret;
}

platform_error sdl_write_stdout(string str) {

	platform_error ret;

	return ret;
}

void* sdl_heap_alloc(u64 bytes) {

	return null;
}

void* sdl_heap_realloc(void* mem, u64 bytes) {

	return null;
}

void sdl_heap_free(void* mem) {

}

platform_error sdl_get_bin_path(string* path) {

	platform_error ret;

	return ret;
}

platform_error sdl_create_thread(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended) {

	platform_error ret;

	return ret;
}

platform_thread_join_state sdl_join_thread(platform_thread* thread, i32 ms) {

	platform_thread_join_state ret;

	return ret;
}

platform_error sdl_destroy_thread(platform_thread* thread) {

	platform_error ret;

	return ret;
}

platform_thread_id sdl_this_thread_id() {

	platform_thread_id ret;

	return ret;
}

platform_error sdl_terminate_thread(platform_thread* thread, i32 exit_code) {

	platform_error ret;

	return ret;
}

void sdl_exit_this_thread(i32 exit_code) {

}

void sdl_thread_sleep(i32 ms) {

}

i32 sdl_get_num_cpus() {

	return 0; 
}

platform_error sdl_create_semaphore(platform_semaphore* sem, i32 initial_count, i32 max_count) {

	platform_error ret;

	return ret;
}

platform_error sdl_destroy_semaphore(platform_semaphore* sem) {

	platform_error ret;

	return ret;
}

platform_semaphore_state sdl_wait_semaphore(platform_semaphore* sem, i32 ms) {

	platform_semaphore_state ret;

	return ret;
}

platform_error sdl_signal_semaphore(platform_semaphore* sem, i32 times) {

	platform_error ret;

	return ret;
} 

void sdl_create_mutex(platform_mutex* mut, bool aquire) {

}

void sdl_destroy_mutex(platform_mutex* mut) {

}

void sdl_aquire_mutex(platform_mutex* mut) {

}

void sdl_try_aquire_mutex(platform_mutex* mut) {

}

void sdl_release_mutex(platform_mutex* mut) {

}

string sdl_make_timef(string fmt) {

	string ret;

	return ret;
}

void sdl_get_timef(string fmt, string* out) {

}
