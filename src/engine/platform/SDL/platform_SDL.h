
#pragma once

bool sdl_window_focused(platform_window* win);
void sdl_capture_mouse(platform_window* win);
void sdl_release_mouse(platform_window* win);
platform_error sdl_set_cursor_pos(platform_window* win, i32 x, i32 y);

u64 sdl_get_perfcount();
u64 sdl_get_perfcount_freq();

bool sdl_is_debugging();
void sdl_debug_break();
void sdl_set_cursor(cursors c);

platform_error sdl_create_window(platform_window* window, string title, u32 width, u32 height);
platform_error sdl_destroy_window(platform_window* window);
platform_error sdl_get_window_size(platform_window* window, i32* w, i32* h);

platform_error sdl_swap_buffers(platform_window* window);

void		   sdl_set_queue_callback(void (*enqueue)(void* queue_param, platform_event evt), void* queue_param);
void 		   sdl_pump_events(platform_window* window);
void		   sdl_queue_event(platform_event evt);
platform_error sdl_wait_message();
bool 		   sdl_keydown(platform_keycode key);

platform_error sdl_load_library(platform_dll* dll, string file_path);
platform_error sdl_free_library(platform_dll* dll);

platform_error sdl_get_proc_address(void** address, platform_dll* dll, string name);
void*		   sdl_get_glproc(string name);

platform_error sdl_copy_file(string source, string dest, bool overwrite);
platform_error sdl_get_file_attributes(platform_file_attributes* attrib, string file_path);
bool 		   sdl_test_file_written(platform_file_attributes* first, platform_file_attributes* second);
platform_error sdl_create_file(platform_file* file, string path, platform_file_open_op mode);
platform_error sdl_close_file(platform_file* file);
platform_error sdl_write_file(platform_file* file, void* mem, u32 bytes);
platform_error sdl_read_file(platform_file* file, void* mem, u32 bytes);
u32			   sdl_file_size(platform_file* file);
platform_error sdl_write_stdout_str(string str);
platform_error sdl_write_stdout(void* mem, u32 len);

// if this fails, we're having big problems
// NOTE(max): heap alloc should return zero-initialized memory
void*	sdl_heap_alloc(u64 bytes); 
void*	sdl_heap_realloc(void* mem, u64 bytes);
void	sdl_heap_free(void* mem);
void*	sdl_heap_alloc_net(u64 bytes);
void	sdl_heap_free_net(void* mem);

// allocates a string
platform_error sdl_get_bin_path(string* path);

platform_error     sdl_create_thread(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended);
platform_error	   sdl_destroy_thread(platform_thread* thread);
platform_thread_id sdl_this_thread_id();
void			   sdl_thread_sleep(i32 ms);
i32  			   sdl_get_num_cpus();
i32  			   sdl_get_phys_cpus();
platform_thread_join_state sdl_join_thread(platform_thread* thread, i32 ms); // ms = -1 for infinite

platform_error sdl_create_semaphore(platform_semaphore* sem, i32 initial_count, i32 max_count);
platform_error sdl_destroy_semaphore(platform_semaphore* sem);
platform_error sdl_signal_semaphore(platform_semaphore* sem, i32 times); 
platform_semaphore_state sdl_wait_semaphore(platform_semaphore* sem, i32 ms); // ms = -1 for infinite

u64 sdl_atomic_exchange(u64* dest, u64 val);
void sdl_create_mutex(platform_mutex* mut, bool aquire);
void sdl_destroy_mutex(platform_mutex* mut);
bool sdl_try_aquire_mutex(platform_mutex* mut);
void sdl_aquire_mutex(platform_mutex* mut);
void sdl_release_mutex(platform_mutex* mut);

// allocates a string
string sdl_time_string();

