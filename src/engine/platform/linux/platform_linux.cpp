// Error Code 125 - Cannont connect to X server
// Error Code 126 - No visual found

#include "platform_linux.h"

// #include <X11/X.h> // Included in X11/Xlib.h, but written here for understanding
// #include <X11/Xlib.h>

platform_api platform_build_api() {

	platform_api ret;

	ret.get_perfcount			= &linux_get_perfcount;
	ret.get_perfcount_freq		= &linux_get_perfcount_freq;
	ret.create_window			= &linux_create_window;
	ret.destroy_window			= &linux_destroy_window;
	ret.swap_buffers			= &linux_swap_buffers;
	ret.set_queue_callback		= &linux_set_queue_callback;
	ret.pump_events				= &linux_pump_events;
	ret.queue_event 			= &linux_queue_event;
	ret.wait_message			= &linux_wait_message;
	ret.load_library			= &linux_load_library;
	ret.free_library			= &linux_free_library;
	ret.get_proc_address 		= &linux_get_proc_address;
	ret.get_file_attributes 	= &linux_get_file_attributes;
	ret.test_file_written		= &linux_test_file_written;
	ret.copy_file				= &linux_copy_file;
	ret.heap_alloc				= &linux_heap_alloc;
	ret.heap_realloc			= &linux_heap_realloc;
	ret.heap_free				= &linux_heap_free;
	ret.get_bin_path			= &linux_get_bin_path;
	ret.create_thread			= &linux_create_thread;
	ret.this_thread_id			= &linux_this_thread_id;
	ret.terminate_thread		= &linux_terminate_thread;
	ret.exit_this_thread		= &linux_exit_this_thread;
	ret.thread_sleep			= &linux_thread_sleep;
	ret.create_semaphore		= &linux_create_semaphore;
	ret.destroy_semaphore		= &linux_destroy_semaphore;
	ret.signal_semaphore		= &linux_signal_semaphore;
	ret.wait_semaphore			= &linux_wait_semaphore;
	ret.create_mutex			= &linux_create_mutex;
	ret.destroy_mutex			= &linux_destroy_mutex;
	ret.aquire_mutex			= &linux_aquire_mutex;
	ret.try_aquire_mutex		= &linux_try_aquire_mutex;
	ret.release_mutex			= &linux_release_mutex;
	ret.destroy_thread			= &linux_destroy_thread;
	ret.get_num_cpus			= &linux_get_num_cpus;
	ret.join_thread				= &linux_join_thread;
	ret.create_file				= &linux_create_file;
	ret.close_file				= &linux_close_file;
	ret.write_file				= &linux_write_file;
	ret.read_file				= &linux_read_file;
	ret.get_stdout_as_file		= &linux_get_stdout_as_file;
	ret.get_timef				= &linux_get_timef;
	ret.make_timef				= &linux_make_timef;
	ret.get_window_size			= &linux_get_window_size;
	ret.write_stdout			= &linux_write_stdout;
	ret.file_size				= &linux_file_size;
	ret.get_glproc				= &linux_get_glproc;
	ret.keydown					= &linux_keydown;
	ret.is_debugging			= &linux_is_debugging;
	ret.debug_break				= &linux_debug_break;
	ret.set_cursor				= &linux_set_cursor;
	ret.this_dll				= &linux_this_dll;
	ret.capture_mouse 			= &linux_capture_mouse;
	ret.release_mouse 			= &linux_release_mouse;
	ret.set_cursor_pos			= &linux_set_cursor_pos;
	ret.window_focused			= &linux_window_focused;
	ret.atomic_exchange			= &linux_atomic_exchange;
	ret.get_phys_cpus 			= &linux_get_phys_cpus;

	return ret;
}

i32 linux_get_phys_cpus() {

	return 0;
}

u64 linux_atomic_exchange(u64* dest, u64 val) {

	return *dest;
}

bool linux_window_focused(platform_window* win) {

	return false;
}

platform_error linux_set_cursor_pos(platform_window* win, i32 x, i32 y) {
	
}

void linux_capture_mouse(platform_window* win) {

}

void linux_release_mouse() {

}

platform_perfcount linux_get_perfcount() {

	return 0;
}

platform_perfcount linux_get_perfcount_freq() {

	return 0;
}

platform_error linux_this_dll(platform_dll* dll) {

	platform_error ret;

	return ret;
}

void linux_debug_break() {

}

bool linux_is_debugging() {
	
	return false;
}

void linux_set_cursor(cursors c) {

}

platform_error linux_create_window(platform_window* window, string title, u32 width, u32 height) {
	platform_error ret;

	Display                 *dpy;
	Window                  root;
	GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo             *vi;
	Colormap                cmap; // ?
	XSetWindowAttributes    swa;
	Window                  win;
	GLXContext              glc;
	XWindowAttributes       gwa;
	XEvent                  xev;

	dpy = XOpenDisplay(NULL); // NULL means that the graphical output is sent to the computer which executes the code
	if (dpy == NULL) {
		platform_error.good = false;
		platform_error.error = 125; // Cannont connect to X server
	}
	root = DefaultRootWindow(dpy);
	vi =glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		platform_error.good = false;
		platform_error.error =  126; // No visual found
	}
	win = XCreateWindow(dpy, root, 0, 0, 600, 600, 0, vi->depth, InputOut, vi->visual, CWColormap | CWEventMask, *&wa);

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

	return null;
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

void linux_create_mutex(platform_mutex* mut, bool aquire) {

}

void linux_destroy_mutex(platform_mutex* mut) {

}

void linux_aquire_mutex(platform_mutex* mut) {

}

bool linux_try_aquire_mutex(platform_mutex* mut) {

	return false;
}

void linux_release_mutex(platform_mutex* mut) {

}

string linux_make_timef(string fmt) {

	string ret;

	return ret;
}

void linux_get_timef(string fmt, string* out) {

}
