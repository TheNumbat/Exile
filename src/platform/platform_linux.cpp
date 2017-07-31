
#include "platform_linux.h"

platform_api platform_build_api() {

	platform_api ret;

	ret.platform_create_window			= &linux_create_window;
	ret.platform_destroy_window			= &linux_destroy_window;
	ret.platform_swap_buffers			= &linux_swap_buffers;
	ret.platform_set_queue_callback		= &linux_set_queue_callback;
	ret.platform_pump_events			= &linux_pump_events;
	ret.platform_queue_event 			= &linux_queue_event;
	ret.platform_wait_message			= &linux_wait_message;
	ret.platform_load_library			= &linux_load_library;
	ret.platform_free_library			= &linux_free_library;
	ret.platform_get_proc_address 		= &linux_get_proc_address;
	ret.platform_get_file_attributes 	= &linux_get_file_attributes;
	ret.platform_test_file_written		= &linux_test_file_written;
	ret.platform_copy_file				= &linux_copy_file;
	ret.platform_heap_alloc				= &linux_heap_alloc;
	ret.platfomr_heap_realloc			= &linux_heap_realloc;
	ret.platform_heap_free				= &linux_heap_free;
	ret.platform_get_bin_path			= &linux_get_bin_path;
	ret.platform_create_thread			= &linux_create_thread;
	ret.platform_this_thread_id			= &linux_this_thread_id;
	ret.platform_terminate_thread		= &linux_terminate_thread;
	ret.platform_exit_this_thread		= &linux_exit_this_thread;
	ret.platform_thread_sleep			= &linux_thread_sleep;
	ret.platform_create_semaphore		= &linux_create_semaphore;
	ret.platform_destroy_semaphore		= &linux_destroy_semaphore;
	ret.platform_signal_semaphore		= &linux_signal_semaphore;
	ret.platform_wait_semaphore			= &linux_wait_semaphore;
	ret.platform_create_mutex			= &linux_create_mutex;
	ret.platform_destroy_mutex			= &linux_destroy_mutex;
	ret.platform_aquire_mutex			= &linux_aquire_mutex;
	ret.platform_release_mutex			= &linux_release_mutex;
	ret.platform_destroy_thread			= &linux_destroy_thread;
	ret.platform_get_num_cpus			= &linux_get_num_cpus;
	ret.platform_join_thread			= &linux_join_thread;
	ret.platform_create_file			= &linux_create_file;
	ret.platform_close_file				= &linux_close_file;
	ret.platform_write_file				= &linux_write_file;
	ret.platform_read_file				= &linux_read_file;
	ret.platform_get_stdout_as_file		= &linux_get_stdout_as_file;
	ret.platform_get_timef				= &linux_get_timef;
	ret.platform_make_timef				= &linux_make_timef;
	ret.platform_get_window_size		= &linux_get_window_size;
	ret.platform_write_stdout			= &linux_write_stdout;
	ret.platform_file_size				= &linux_file_size;
	ret.platform_get_glproc				= &linux_get_glproc;
	ret.platform_keydown				= &linux_keydown;
	ret.platform_is_debugging			= &linux_is_debugging;

	return ret;
}

bool linux_is_debugging() {

	return false;
}

platform_error linux_create_window(platform_window* window, string title, u32 width, u32 height) {

	platform_error ret;

	return ret;
}

platform_error linux_destroy_window(platform_window* window) {

	platform_error ret;

	return ret;
}

platform_error linux_get_window_size(platform_window* window, i32* w, i32* h) {

	platform_error ret;

	return ret;
}

platform_error linux_swap_buffers(platform_window* window) {

	platform_error ret;

	return ret;
}

void linux_set_queue_callback(void (*enqueue)(void* queue_param, platform_event evt), void* queue_param) {

}

void linux_pump_events(platform_window* window) {

}

void linux_queue_event(platform_event evt) {

}

platform_error linux_wait_message() {

	platform_error ret;

	return ret;
}

bool linux_keydown(platform_keycode key) {

	return false;
}

platform_error linux_load_library(platform_dll* dll, string file_path) {

	platform_error ret;

	return ret;
}

platform_error linux_free_library(platform_dll* dll) {

	platform_error ret;

	return ret;
}

platform_error linux_get_proc_address(void** address, platform_dll* dll, string name) {

	platform_error ret;

	return ret;
}

void* linux_get_glproc(string name) {

	return null;
}

platform_error linux_copy_file(string source, string dest, bool overwrite) {

	platform_error ret;

	return ret;
}

platform_error linux_get_file_attributes(platform_file_attributes* attrib, string file_path) {

	platform_error ret;

	return ret;
}

bool linux_test_file_written(platform_file_attributes* first, platform_file_attributes* second) {

	return false;
}

platform_error linux_create_file(platform_file* file, string path, platform_file_open_op mode) {

	platform_error ret;

	return ret;
}

platform_error linux_close_file(platform_file* file) {

	platform_error ret;

	return ret;
}

platform_error linux_write_file(platform_file* file, void* mem, u32 bytes) {

	platform_error ret;

	return ret;
}

platform_error linux_read_file(platform_file* file, void* mem, u32 bytes) {

	platform_error ret;

	return ret;
}

u32	linux_file_size(platform_file* file) {

	return 0;
}

platform_error linux_get_stdout_as_file(platform_file* file) {

	platform_error ret;

	return ret;
}

platform_error linux_write_stdout(string str) {

	platform_error ret;

	return ret;
}

void* linux_heap_alloc(u64 bytes) {

	return null;
}

void* linux_heap_realloc(void* mem, u64 bytes) {

}

void linux_heap_free(void* mem) {

}

platform_error linux_get_bin_path(string* path) {

	platform_error ret;

	return ret;
}

platform_error linux_create_thread(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended) {

	platform_error ret;

	return ret;
}

platform_thread_join_state linux_join_thread(platform_thread* thread, i32 ms) {

	platform_thread_join_state ret;

	return ret;
}

platform_error linux_destroy_thread(platform_thread* thread) {

	platform_error ret;

	return ret;
}

platform_thread_id linux_this_thread_id() {

	platform_thread_id ret;

	return ret;
}

platform_error linux_terminate_thread(platform_thread* thread, i32 exit_code) {

	platform_error ret;

	return ret;
}

void linux_exit_this_thread(i32 exit_code) {

}

void linux_thread_sleep(i32 ms) {

}

i32 linux_get_num_cpus() {

	return 0; 
}

platform_error linux_create_semaphore(platform_semaphore* sem, i32 initial_count, i32 max_count) {

	platform_error ret;

	return ret;
}

platform_error linux_destroy_semaphore(platform_semaphore* sem) {

	platform_error ret;

	return ret;
}

platform_semaphore_state linux_wait_semaphore(platform_semaphore* sem, i32 ms) {

	platform_semaphore_state ret;

	return ret;
}

platform_error linux_signal_semaphore(platform_semaphore* sem, i32 times) {

	platform_error ret;

	return ret;
} 

platform_error linux_create_mutex(platform_mutex* mut, bool aquire) {

	platform_error ret;

	return ret;
}

platform_error linux_destroy_mutex(platform_mutex* mut) {

	platform_error ret;

	return ret;
}

platform_mutex_state linux_aquire_mutex(platform_mutex* mut, i32 ms) {

	platform_mutex_state ret;

	return ret;
}

platform_error linux_release_mutex(platform_mutex* mut) {

	platform_error ret;

	return ret;
}

string linux_make_timef(string fmt) {

	string ret;

	return ret;
}

void linux_get_timef(string fmt, string* out) {

}
