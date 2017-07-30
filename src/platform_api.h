#pragma once

// this file is for both the platform layer and the game - this defines the interface for the game

// Defined 
struct platform_window;
struct platform_dll;
struct platform_file_attributes;
struct platform_thread_id;
struct platform_thread;
struct platform_semaphore;
struct platform_mutex;
struct platform_file;

struct platform_error {
	bool good = true;
	u32 error = 0;
};

enum class platform_file_open_op {
	existing,
	existing_or_create,
	create,
	clear_existing,
};

enum class _platform_semaphore_state {
	signaled,
	timed_out,
	failed,
};

enum class _platform_mutex_state {
	abandoned,
	aquired,
	timed_out,
	failed,
};

enum class _platform_thread_join_state {
	joined,
	timed_out,
	failed,
};

struct platform_semaphore_state {
	// Transparent
	_platform_semaphore_state state;
	platform_error error;
};

struct platform_mutex_state {
	// Transparent
	_platform_mutex_state state;
	platform_error error;
};

struct platform_thread_join_state {
	// Transparent
	_platform_thread_join_state state;
	platform_error error;
};

extern u32 PLATFORM_SHARING_ERROR;

bool operator==(platform_thread_id first, platform_thread_id second);
bool operator==(platform_file first, platform_file second);

// TODO(max): Is this really the best way to handle input? We're basically just translating the OS event system.
//			  Instead, we could do it the HMH way of representing all input since last poll as one structure.

// TODO(max): more event types (e.g. controller, text?)
enum class platform_event_type : u8 {
	none,
	window, // includes quit
	key,
	mouse,
};

enum class platform_keycode : u8 {
	none = 0,
	_0,
	_1,
	_2,
	_3,
	_4,
	_5,
	_6,
	_7,
	_8,
	_9,
	a,
	b,
	c,
	d,
	e,
	f,
	g,
	h,
	i,
	j,
	k,
	l,
	m,
	n,
	o,
	p,
	q,
	r,
	s,
	t,
	u,
	v,
	w,
	x,
	y,
	z,
	tab,
	grave,
	comma,
	slash,
	space,
	equals,
	plus = equals,
	enter,
	period,
	dash,
	minus = dash,
	rbracket,
	lbracket,
	semicolon,
	backslash,
	np_0,
	np_1,
	np_2,
	np_3,
	np_4,
	np_5,
	np_6,
	np_7,
	np_8,
	np_9,
	np_add,
	np_period,
	np_divide,
	np_multiply,
	np_subtract,
	backspace,
	capslock,
	del,
	down,
	up,
	left,
	right,
	end,
	escape,
	f1,
	f2,
	f3,
	f4,
	f5,
	f6,
	f7,
	f8,
	f9,
	f10,
	f11,
	f12,
	home,
	insert,
	lalt,
	ralt,
	lctrl,
	rctrl,
	lshift,
	rshift,
	numlock,
	pgup,
	pgdown,
	scrolllock,
};

enum class platform_keyflag : u16 {
	press       = 1<<0,
	release     = 1<<1,
	repeat      = 1<<2,

	lshift      = 1<<3,
	rshift      = 1<<4,
	shift       = rshift | lshift,
	lctrl       = 1<<5,
	rctrl       = 1<<6,
	ctrl        = rctrl | lctrl,
	lalt        = 1<<7,
	ralt        = 1<<8,
	alt         = ralt | lalt,

	numlock_on  = 1<<9,
	capslock_on = 1<<10,
};

enum class platform_mouseflag : u16 {
	press   = 1<<0,
	release = 1<<1,
	wheel   = 1<<2,
	dbl     = 1<<3,
	button  = press | release | dbl,
	lclick  = 1<<4,
	rclick  = 1<<5,
	mclick  = 1<<6,
	x1click = 1<<7,
	x2click = 1<<8,
	click   = rclick | lclick | mclick | x1click | x2click,
	move    = 1<<9,
};

// TODO(max): cursors
enum class platform_windowop : u8 {
	none,
	resized,	// WM_SIZE
	moved,		// WM_MOVE
	shown,		// WM_SHOWWINDOW
	hidden,		// WM_SHOWWINDOW
	minimized,	// WM_SHOWWINDOW, WM_SIZE
	maximized,	// WM_SIZE
	restored,	// WM_SHOWWINDOW
	focused,	// WM_ACTIVATEAPP
	unfocused,	// WM_ACTIVATEAPP
	close,		// WM_CLOSE, WM_DESTROY, WM_QUIT
};

struct _platform_event_window {
	platform_windowop op = platform_windowop::none;
	i16 x = 0;
	i16 y = 0;
};

struct _platform_event_key {
	platform_keycode code = platform_keycode::none;
	u16 flags = 0;
};

struct _platform_event_mouse {
	i16 x = 0;
	i16 y = 0;
	u16 flags = 0;
	i8 w = 0;
};

struct platform_event {
	platform_event_type type = platform_event_type::none;
	union { 
		// no data for platform_event_quit
		_platform_event_window		window;
		_platform_event_key 		key;
		_platform_event_mouse 		mouse;
	};
	platform_event() : type(), window(), key(), mouse() {}; // c++ reee
};

struct platform_api {
	bool 			(*platform_is_debugging)()																					= NULL;
	void 			(*platform_set_queue_callback)(void (*enqueue)(void* queue_param, platform_event evt), void* queue_param)	= NULL;
	void 			(*platform_queue_messages)(platform_window* window)															= NULL;
	bool 		    (*platform_keydown)(platform_keycode key)																	= NULL;
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
	void* 			(*platform_heap_realloc)(void* mem, u64 bytes)																= NULL;
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

#include "platform_api_win32.h"

#elif defined(__linux__)

#include "platform_api_linux.h"

#else

#error "Unsupported platform."

#endif