
#include "platform_SDL.h"

#include <SDL2/SDL.h>

platform_api platform_build_api() {

	platform_api ret;

	ret.get_perfcount			= &sdl_get_perfcount;
	ret.get_perfcount_freq		= &sdl_get_perfcount_freq;
	ret.create_window			= &sdl_create_window;
	ret.destroy_window			= &sdl_destroy_window;
	ret.swap_buffers			= &sdl_swap_buffers;
	ret.set_queue_callback		= &sdl_set_queue_callback;
	ret.pump_events				= &sdl_pump_events;
	ret.queue_event 			= &sdl_queue_event;
	ret.wait_message			= &sdl_wait_message;
	ret.load_library			= &sdl_load_library;
	ret.free_library			= &sdl_free_library;
	ret.get_proc_address 		= &sdl_get_proc_address;
	ret.get_file_attributes 	= &sdl_get_file_attributes;
	ret.test_file_written		= &sdl_test_file_written;
	ret.copy_file				= &sdl_copy_file;
	ret.heap_alloc				= &sdl_heap_alloc;
	ret.heap_realloc			= &sdl_heap_realloc;
	ret.heap_free				= &sdl_heap_free;
	ret.get_bin_path			= &sdl_get_bin_path;
	ret.create_thread			= &sdl_create_thread;
	ret.this_thread_id			= &sdl_this_thread_id;
	ret.terminate_thread		= &sdl_terminate_thread;
	ret.exit_this_thread		= &sdl_exit_this_thread;
	ret.thread_sleep			= &sdl_thread_sleep;
	ret.create_semaphore		= &sdl_create_semaphore;
	ret.destroy_semaphore		= &sdl_destroy_semaphore;
	ret.signal_semaphore		= &sdl_signal_semaphore;
	ret.wait_semaphore			= &sdl_wait_semaphore;
	ret.create_mutex			= &sdl_create_mutex;
	ret.destroy_mutex			= &sdl_destroy_mutex;
	ret.aquire_mutex			= &sdl_aquire_mutex;
	ret.try_aquire_mutex		= &sdl_try_aquire_mutex;
	ret.release_mutex			= &sdl_release_mutex;
	ret.destroy_thread			= &sdl_destroy_thread;
	ret.get_num_cpus			= &sdl_get_num_cpus;
	ret.join_thread				= &sdl_join_thread;
	ret.create_file				= &sdl_create_file;
	ret.close_file				= &sdl_close_file;
	ret.write_file				= &sdl_write_file;
	ret.read_file				= &sdl_read_file;
	ret.get_stdout_as_file		= &sdl_get_stdout_as_file;
	ret.get_timef				= &sdl_get_timef;
	ret.make_timef				= &sdl_make_timef;
	ret.get_window_size			= &sdl_get_window_size;
	ret.write_stdout			= &sdl_write_stdout;
	ret.file_size				= &sdl_file_size;
	ret.get_glproc				= &sdl_get_glproc;
	ret.keydown					= &sdl_keydown;
	ret.is_debugging			= &sdl_is_debugging;
	ret.debug_break				= &sdl_debug_break;
	ret.set_cursor				= &sdl_set_cursor;
	ret.this_dll				= &sdl_this_dll;
	ret.capture_mouse			= &sdl_capture_mouse;
	ret.release_mouse			= &sdl_release_mouse;
	ret.set_cursor_pos			= &sdl_set_cursor_pos;
	ret.atomic_exchange 		= &sdl_atomic_exchange;
	ret.window_focused 			= &sdl_window_focused;
	ret.get_phys_cpus			= &sdl_get_phys_cpus;

	return ret;
}

i32 sdl_get_phys_cpus() {
	return 0;
}

u64 sdl_atomic_exchange(u64* dest, u64 val) {

	return *dest;
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

bool sdl_try_aquire_mutex(platform_mutex* mut) {

	return false;
}

void sdl_release_mutex(platform_mutex* mut) {

}

string sdl_make_timef(string fmt) {

	string ret;

	return ret;
}

void sdl_get_timef(string fmt, string* out) {

}
