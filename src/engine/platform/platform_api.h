/* Included in platform_main.cpp and the game code */

#pragma once

#define EXTERN extern "C"

#ifdef _MSC_VER
#define DLL_EXPORT EXTERN __declspec(dllexport)
#define DLL_IMPORT EXTERN __declspec(dllimport)
#elif defined(__GNUC__)
#define DLL_EXPORT EXTERN __attribute__((dllexport))
#define DLL_IMPORT EXTERN __attribute__((dllimport))
#else
#error unsupported compiler?
#endif

// Defined per platform
struct platform_window_internal;
struct platform_dll;
struct platform_file_attributes;
struct platform_thread;
struct platform_semaphore;
struct platform_mutex;
struct platform_file;
typedef u32 platform_thread_id;
extern u32 PLATFORM_SHARING_ERROR;
bool operator==(platform_file first, platform_file second);

#ifdef PLATFORM_SDL
#include "sdl/platform_sdl_api.h"
#elif defined(_WIN32)
#include "windows/platform_win32_api.h"
#else
#error Unsupported Platform
#endif

enum platform_window_mode : u8 {
	windowed,
	fullscreen,
};

#define WINDOW_TITLE_LEN 128
struct platform_window_settings {

	char c_title[WINDOW_TITLE_LEN] = {};

	platform_window_mode mode = platform_window_mode::windowed;
	i32 w = 1280, h = 720;
	i32 samples = 1;
	bool vsync = false;
};

struct platform_window {
	platform_window_settings settings;
	platform_window_internal internal;
};

struct platform_error {
	bool good = true;
	u32 error = 0;
	string error_message;
};

enum class platform_file_open_op : u8 {
	existing,
	cleared
};

enum class _platform_semaphore_state : u8 {
	signaled,
	timed_out,
	failed,
};

enum class _platform_thread_join_state : u8 {
	joined,
	timed_out,
	failed,
};

struct platform_semaphore_state {
	// Transparent
	_platform_semaphore_state state;
	platform_error error;
};

struct platform_thread_join_state {
	// Transparent
	_platform_thread_join_state state;
	platform_error error;
};

// TODO(max): Is this really the best way to handle input? We're basically just translating the OS event system.
//			  Instead, we could do it the HMH way of representing all input since last poll as one structure.

// TODO(max): controller events 
enum class platform_event_type : u8 {
	none,
	window,
	key,
	mouse,
	rune,
	async
};

#ifndef RENAME
#define RENAME(name)
#endif

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

enum class platform_cursor : u8 {
	none,
	pointer,
	crosshair,
	hand,
	help,
	I,
	hourglass
};

enum class platform_windowop : u8 {
	none,
	recreate,
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

struct _platform_event_rune {
	char rune_utf8[5] = {};
};

enum class platform_async_type : u8 {
	none,
	user,
};

struct _platform_event_async {
	platform_async_type type = platform_async_type::none;
};

struct platform_event {
	platform_event_type type = platform_event_type::none;
	union { 
		_platform_event_window		window;
		_platform_event_key 		key;
		_platform_event_mouse 		mouse;
		_platform_event_rune 		rune;
		_platform_event_async 		async;
	};
	platform_event() : type(), window() {}; // c++ reee
};

struct platform_api {

	platform_dll* your_dll;

	bool 		   (*window_focused)(platform_window* win);	
	platform_error (*swap_buffers)(platform_window* window);
	platform_error (*create_window)(platform_window* window);
	platform_error (*destroy_window)(platform_window* window);
	platform_error (*recreate_window)(platform_window* window);
	bool 		   (*apply_window_settings)(platform_window* window); // returns whether or not the window (and OpenGL context stuff) needs to be recreated
	platform_error (*get_window_size)(platform_window* window, i32* w, i32* h);
	platform_error (*get_window_drawable)(platform_window* window, i32* w, i32* h);
	
	string 		   (*get_clipboard)();
	void 		   (*set_clipboard)(string text);

	bool 		   (*cursor_shown)();
	void 		   (*show_cursor)(bool show);
	void 		   (*capture_mouse)(platform_window* win);
	void 		   (*release_mouse)(platform_window* win);
	platform_error (*set_cursor_pos)(platform_window* win, i32 x, i32 y);
	void 		   (*set_cursor)(platform_window* win, platform_cursor c);
	platform_error (*get_cursor_pos)(platform_window* win, i32* x, i32* y);
	bool 		   (*mousedown)(platform_mouseflag button);
	
	void 		   (*debug_break)();
	bool 		   (*is_debugging)();
	u64 		   (*get_perfcount)();
	u64 		   (*get_perfcount_freq)();
	
	platform_error (*wait_message)();
	void 		   (*queue_event)(platform_event evt);
	void 		   (*pump_events)(platform_window* window);
	void 		   (*set_queue_callback)(void (*enqueue)(void* queue_param, platform_event evt), void* queue_param);
	
	bool 		   (*keydown)(platform_keycode key);
	i32 		   (*get_scancode)(platform_keycode code);
	
	void*		   (*get_glproc)(string name);
	platform_error (*free_library)(platform_dll* dll);
	platform_error (*load_library)(platform_dll* dll, string file_path);
	platform_error (*get_proc_address)(void** address, platform_dll* dll, string name);

	void	  	   (*heap_free)(void* mem);
	void*		   (*heap_alloc)(u64 bytes);
	void* 		   (*heap_realloc)(void* mem, u64 bytes);
	
	platform_thread_id  	   (*this_thread_id)();
	void		   			   (*thread_sleep)(i32 ms);
	u64 					   (*atomic_exchange)(u64* dest, u64 val);
	platform_error 			   (*destroy_thread)(platform_thread* thread);
	platform_thread_join_state (*join_thread)(platform_thread* thread, i32 ms);
	platform_error 			   (*create_thread)(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended);

	platform_error 			 (*destroy_semaphore)(platform_semaphore* sem);
	platform_error 			 (*signal_semaphore)(platform_semaphore* sem, i32 times);
	platform_error 			 (*create_semaphore)(platform_semaphore* sem, i32 initial_count, i32 max_count);
	platform_semaphore_state (*wait_semaphore)(platform_semaphore* sem, i32 ms);

	void 		   (*aquire_mutex)(platform_mutex* mut);
	void 		   (*release_mutex)(platform_mutex* mut);
	void 		   (*destroy_mutex)(platform_mutex* mut);
	bool 		   (*try_aquire_mutex)(platform_mutex* mut);
	void 		   (*create_mutex)(platform_mutex* mut, bool aquire);
	
	i32   		   (*get_num_cpus)();
	i32 		   (*get_phys_cpus)();
	
	platform_error (*get_bin_path)(string* path); // allocates
	u32			   (*file_size)(platform_file* file);
	platform_error (*close_file)(platform_file* file);
	platform_error (*read_file)(platform_file* file, void* mem, u32 bytes);
	platform_error (*write_file)(platform_file* file, void* mem, u32 bytes);
	platform_error (*copy_file)(string source, string dest, bool overwrite);
	platform_error (*get_file_attributes)(platform_file_attributes* attrib, string file_path);
	platform_error (*create_file)(platform_file* file, string path, platform_file_open_op mode);
	bool 		   (*test_file_written)(platform_file_attributes* first, platform_file_attributes* second);

	platform_error (*shell_exec)(string cmd);
	platform_error (*write_stdout_str)(string str);
	platform_error (*write_stdout)(void* mem, u32 len);	
	
	string 		   (*time_string)();
};

// NOTE(max): for "main"
platform_api platform_build_api();
void platform_test_api();
void platform_shutdown();

#include "gl.h"
