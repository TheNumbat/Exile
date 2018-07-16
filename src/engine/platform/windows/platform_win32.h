
#pragma once

/*
void platform_shutdown();
void platform_test_api()
platform_api platform_build_api();
*/

bool 		   win32_window_focused(platform_window* win);	
platform_error win32_swap_buffers(platform_window* window);
platform_error win32_create_window(platform_window* window);
platform_error win32_destroy_window(platform_window* window);
platform_error win32_recreate_window(platform_window* window);
bool 		   win32_apply_window_settings(platform_window* window);
platform_error win32_get_window_size(platform_window* window, i32* w, i32* h);
platform_error win32_get_window_drawable(platform_window* window, i32* w, i32* h);

string 		   win32_get_clipboard();
void 		   win32_set_clipboard(string text);

bool 		   win32_cursor_shown();
void 		   win32_show_cursor(bool show);
void 		   win32_set_cursor(platform_window* win, platform_cursor c);
void 		   win32_capture_mouse(platform_window* win);
void 		   win32_release_mouse(platform_window* win);
platform_error win32_set_cursor_pos(platform_window* win, i32 x, i32 y);
platform_error win32_get_cursor_pos(platform_window* win, i32* x, i32* y);
bool 		   win32_mousedown(platform_mouseflag button);
	
void 		   win32_debug_break();
bool 		   win32_is_debugging();
u64 		   win32_get_perfcount();
u64 		   win32_get_perfcount_freq();
	
platform_error win32_wait_message();
void 		   win32_queue_event(platform_event evt);
void 		   win32_pump_events(platform_window* window);
void 		   win32_set_queue_callback(void win32_enqueue(void* queue_param, platform_event evt), void* queue_param);
	
bool 		   win32_keydown(platform_keycode key);
i32 		   win32_get_scancode(platform_keycode code);
	
void*		   win32_get_glproc(string name);
platform_error win32_free_library(platform_dll* dll);
platform_error win32_load_library(platform_dll* dll, string file_path);
platform_error win32_get_proc_address(void** address, platform_dll* dll, string name);

void	  	   win32_heap_free(void* mem);
void*		   win32_heap_alloc(u64 bytes);
void* 		   win32_heap_realloc(void* mem, u64 bytes);
	
platform_thread_id  	   win32_this_thread_id();
void		   			   win32_thread_sleep(i32 ms);
u64 					   win32_atomic_exchange(u64* dest, u64 val);
platform_error 			   win32_destroy_thread(platform_thread* thread);
platform_thread_join_state win32_join_thread(platform_thread* thread, i32 ms);
platform_error 			   win32_create_thread(platform_thread* thread, i32 win32_proc(void*), void* param, bool start_suspended);

platform_error 			 win32_destroy_semaphore(platform_semaphore* sem);
platform_error 			 win32_signal_semaphore(platform_semaphore* sem, i32 times);
platform_error 			 win32_create_semaphore(platform_semaphore* sem, i32 initial_count, i32 max_count);
platform_semaphore_state win32_wait_semaphore(platform_semaphore* sem, i32 ms);

void 		   win32_aquire_mutex(platform_mutex* mut);
void 		   win32_release_mutex(platform_mutex* mut);
void 		   win32_destroy_mutex(platform_mutex* mut);
bool 		   win32_try_aquire_mutex(platform_mutex* mut);
void 		   win32_create_mutex(platform_mutex* mut, bool aquire);
	
i32   		   win32_get_num_cpus();
i32 		   win32_get_phys_cpus();
	
platform_error win32_get_bin_path(string* path); // allocates
u32			   win32_file_size(platform_file* file);
platform_error win32_close_file(platform_file* file);
platform_error win32_read_file(platform_file* file, void* mem, u32 bytes);
platform_error win32_write_file(platform_file* file, void* mem, u32 bytes);
platform_error win32_copy_file(string source, string dest, bool overwrite);
platform_error win32_get_file_attributes(platform_file_attributes* attrib, string file_path);
platform_error win32_create_file(platform_file* file, string path, platform_file_open_op mode);
bool 		   win32_test_file_written(platform_file_attributes* first, platform_file_attributes* second);

platform_error win32_shell_exec(string cmd);
platform_error win32_write_stdout_str(string str);
platform_error win32_write_stdout(void* mem, u32 len);	
	
string 		   win32_time_string();

