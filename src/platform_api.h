#pragma once

// this file is for both the platform layer and the game - this defines the interface for the game

struct platform_window;
struct platform_error;
struct platform_dll;
struct platform_file_attributes;
struct platform_thread_id;
struct platform_thread;
struct platform_semaphore;
struct platform_mutex;
struct platform_semaphore_state;
struct platform_mutex_state;
struct platform_thread_join_state;
struct platform_file;

bool operator==(platform_thread_id& first, platform_thread_id& second);

// TODO(max): Is this really the best way to handle input? We're basically just translating the OS event system.
//			  Instead, we could do it the HMH way of representing all input since last poll as one structure.

// TODO(max): more event types (e.g. controller, text?)
enum platform_event_type : u8 {
	event_none,
	event_window, // includes quit
	event_key,
	event_mouse,
};

enum platform_keycode : u8 {
	key_none = 0,
	key_0,
	key_1,
	key_2,
	key_3,
	key_4,
	key_5,
	key_6,
	key_7,
	key_8,
	key_9,
	key_a,
	key_b,
	key_c,
	key_d,
	key_e,
	key_f,
	key_g,
	key_h,
	key_i,
	key_j,
	key_k,
	key_l,
	key_m,
	key_n,
	key_o,
	key_p,
	key_q,
	key_r,
	key_s,
	key_t,
	key_u,
	key_v,
	key_w,
	key_x,
	key_y,
	key_z,
	key_tab,
	key_grave,
	key_comma,
	key_slash,
	key_space,
	key_equals,
	key_enter,
	key_period,
	key_dash,
	key_rbracket,
	key_lbracket,
	key_semicolon,
	key_backslash,
	key_np_0,
	key_np_1,
	key_np_2,
	key_np_3,
	key_np_4,
	key_np_5,
	key_np_6,
	key_np_7,
	key_np_8,
	key_np_9,
	key_np_add,
	key_np_period,
	key_np_divide,
	key_np_multiply,
	key_np_subtract,
	key_backspace,
	key_capslock,
	key_delete,
	key_down,
	key_up,
	key_left,
	key_right,
	key_end,
	key_escape,
	key_f1,
	key_f2,
	key_f3,
	key_f4,
	key_f5,
	key_f6,
	key_f7,
	key_f8,
	key_f9,
	key_f10,
	key_f11,
	key_f12,
	key_home,
	key_insert,
	key_lalt,
	key_ralt,
	key_lctrl,
	key_rctrl,
	key_lshift,
	key_rshift,
	key_numlock,
	key_pgup,
	key_pgdown,
	key_scrolllock,
};

enum platform_keyflag : u16 {
	key_flag_press = 1<<0,
	key_flag_release = 1<<1,
	key_flag_repeat = 1<<2,

	key_flag_lshift = 1<<3,
	key_flag_rshift = 1<<4,
	key_flag_shift = key_flag_rshift | key_flag_lshift,
	key_flag_lctrl = 1<<5,
	key_flag_rctrl = 1<<6,
	key_flag_ctrl = key_flag_rctrl | key_flag_lctrl,
	key_flag_lalt = 1<<7,
	key_flag_ralt = 1<<8,
	key_flag_alt = key_flag_ralt | key_flag_lalt,

	key_flag_numlock_on = 1<<9,
	key_flag_capslock_on = 1<<10,
};

enum platform_mouseflag : u16 {
	mouse_flag_press = 1<<0,
	mouse_flag_release = 1<<1,
	mouse_flag_wheel = 1<<2,
	mouse_flag_double = 1<<3,
	mouse_flag_button = mouse_flag_press | mouse_flag_release | mouse_flag_double,
	mouse_flag_lclick = 1<<4,
	mouse_flag_rclick = 1<<5,
	mouse_flag_mclick = 1<<6,
	mouse_flag_x1click = 1<<7,
	mouse_flag_x2click = 1<<8,
	mouse_flag_click = mouse_flag_rclick | mouse_flag_lclick | mouse_flag_mclick | mouse_flag_x1click | mouse_flag_x2click,
	mouse_flag_move = 1<<9,
};

// TODO(max): cursors
enum platform_windowop : u8 {
	window_none,
	window_resized,		// WM_SIZE
	window_moved,		// WM_MOVE
	window_shown,		// WM_SHOWWINDOW
	window_hidden,		// WM_SHOWWINDOW
	window_minimized,	// WM_SHOWWINDOW, WM_SIZE
	window_maximized,	// WM_SIZE
	window_restored,	// WM_SHOWWINDOW
	window_focused,		// WM_ACTIVATEAPP
	window_unfocused,	// WM_ACTIVATEAPP
	window_close,		// WM_CLOSE, WM_DESTROY, WM_QUIT
};

struct _platform_event_window {
	platform_windowop op = window_none;
	i16 x = 0, y = 0;
};

struct _platform_event_key {
	platform_keycode code = key_none;
	u16 flags = 0;
};

struct _platform_event_mouse {
	i16 x = 0, y = 0;
	u16 flags = 0;
	u8 w = 0;
};

struct platform_event {
	platform_event_type type = event_none;
	union { 
		// no data for platform_event_quit
		_platform_event_window		window;
		_platform_event_key 		key;
		_platform_event_mouse 		mouse;
	};
	platform_event() : type(), window(), key(), mouse() {}; // c++ reee
};

enum platform_file_open_op {
	open_file_existing,
	open_file_existing_or_create,
	open_file_create,
	open_file_clear_existing,
};

enum _platform_semaphore_state {
	semaphore_signaled,
	semaphore_timed_out,
	semaphore_failed,
};

enum _platform_mutex_state {
	mutex_abandoned,
	mutex_aquired,
	mutex_timed_out,
	mutex_failed,
};

enum _platform_thread_join_state {
	thread_joined,
	thread_timed_out,
	thread_failed,
};

struct platform_api {
	void 			(*platform_set_queue_callback)(void (*enqueue)(void* queue_param, platform_event evt), void* queue_param)	= NULL;
	void 			(*platform_queue_messages)(platform_window* window)															= NULL;
	platform_error 	(*platform_create_window)(platform_window* window, string title, u32 width, u32 height)						= NULL;
	platform_error 	(*platform_destroy_window)(platform_window* window)															= NULL;
	platform_error 	(*platform_get_window_size)(platform_window* window, i32* w, i32* h)										= NULL;
	platform_error 	(*platform_swap_buffers)(platform_window* window)															= NULL;
	platform_error 	(*platform_wait_message)()																					= NULL;
	platform_error 	(*platform_load_library)(platform_dll* dll, string file_path)												= NULL;
	platform_error 	(*platform_free_library)(platform_dll* dll)																	= NULL;
	platform_error 	(*platform_get_proc_address)(void** address, platform_dll* dll, string name)								= NULL;
	void*		   	(*platform_get_glproc)(string name)																			= NULL;
	platform_error 	(*platform_get_file_attributes)(platform_file_attributes* attrib, string file_path)							= NULL;
	bool 		   	(*platform_test_file_written)(platform_file_attributes* first, platform_file_attributes* second) 			= NULL;
	platform_error 	(*platform_copy_file)(string source, string dest, bool overwrite)											= NULL;
	void*		   	(*platform_heap_alloc)(u64 bytes)																			= NULL;
	void	  	   	(*platform_heap_free)(void* mem)																			= NULL;
	platform_error 	(*platform_get_bin_path)(string* path) /* heap_allocs a string */											= NULL;
	platform_error 	(*platform_create_thread)(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended)	= NULL;
	platform_thread_id (*platform_this_thread_id)()																				= NULL;
	platform_error 	(*platform_terminate_thread)(platform_thread* thread, i32 exit_code)										= NULL;
	void 		   	(*platform_exit_this_thread)(i32 exit_code)																	= NULL;
	void		   	(*platform_thread_sleep)(i32 ms)																			= NULL;
	platform_error 	(*platform_destroy_thread)(platform_thread* thread)															= NULL;
	platform_error 	(*platform_create_semaphore)(platform_semaphore* sem, i32 initial_count, i32 max_count)						= NULL;
	platform_error 	(*platform_destroy_semaphore)(platform_semaphore* sem)														= NULL;
	platform_error 	(*platform_signal_semaphore)(platform_semaphore* sem, i32 times)											= NULL;
	platform_semaphore_state (*platform_wait_semaphore)(platform_semaphore* sem, i32 ms)										= NULL;
	platform_error 	(*platform_create_mutex)(platform_mutex* mut, bool aquire)													= NULL;
	platform_error 	(*platform_destroy_mutex)(platform_mutex* mut)																= NULL;
	platform_mutex_state (*platform_aquire_mutex)(platform_mutex* mut, i32 ms)													= NULL;
	platform_error 	(*platform_release_mutex)(platform_mutex* mut)																= NULL;
	i32   		   	(*platform_get_num_cpus)()																					= NULL;
	platform_thread_join_state (*platform_join_thread)(platform_thread* thread, i32 ms)											= NULL;
	platform_error 	(*platform_create_file)(platform_file* file, string path, platform_file_open_op mode)						= NULL;
	platform_error 	(*platform_close_file)(platform_file* file)																	= NULL;
	platform_error 	(*platform_write_file)(platform_file* file, void* mem, u32 bytes)											= NULL;
	platform_error 	(*platform_read_file)(platform_file* file, void* mem, u32 bytes)											= NULL;
	platform_error 	(*platform_get_stdout_as_file)(platform_file* file)															= NULL;
	platform_error 	(*platform_write_stdout)(string str)																		= NULL;
	string 			(*platform_make_timef)(string fmt)																			= NULL;
	void 			(*platform_get_timef)(string fmt, string* out)																= NULL;
	u32			   	(*platform_file_size)(platform_file* file)																	= NULL;
};

#ifdef _WIN32

#include "platform_win32_api.h"

#else

#error "Unsupported platform."

#endif
