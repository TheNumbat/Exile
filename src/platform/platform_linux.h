
#pragma once

platform_api   platform_build_api();

bool linux_is_debugging();
void linux_debug_break();
void linux_set_cursor(cursors c);

platform_error linux_create_window(platform_window* window, string title, u32 width, u32 height);
platform_error linux_destroy_window(platform_window* window);
platform_error linux_get_window_size(platform_window* window, i32* w, i32* h);

platform_error linux_swap_buffers(platform_window* window);

void		   linux_set_queue_callback(void (*enqueue)(void* queue_param, platform_event evt), void* queue_param);
void 		   linux_pump_events(platform_window* window);
void		   linux_queue_event(platform_event evt);
platform_error linux_wait_message();
bool 		   linux_keydown(platform_keycode key);

platform_error linux_this_dll(platform_dll* dll);
platform_error linux_load_library(platform_dll* dll, string file_path);
platform_error linux_free_library(platform_dll* dll);

platform_error linux_get_proc_address(void** address, platform_dll* dll, string name);
void*		   linux_get_glproc(string name);

platform_error linux_copy_file(string source, string dest, bool overwrite);
platform_error linux_get_file_attributes(platform_file_attributes* attrib, string file_path);
bool 		   linux_test_file_written(platform_file_attributes* first, platform_file_attributes* second);
platform_error linux_create_file(platform_file* file, string path, platform_file_open_op mode);
platform_error linux_close_file(platform_file* file);
platform_error linux_write_file(platform_file* file, void* mem, u32 bytes);
platform_error linux_read_file(platform_file* file, void* mem, u32 bytes);
u32			   linux_file_size(platform_file* file);
platform_error linux_get_stdout_as_file(platform_file* file);
platform_error linux_write_stdout(string str);

// if this fails, we're having big problems
void*	linux_heap_alloc(u64 bytes); // initializes memory to zero (important! the data structures assume this!)
void*	linux_heap_realloc(void* mem, u64 bytes);
void	linux_heap_free(void* mem);

// allocates a string
platform_error linux_get_bin_path(string* path);

platform_error     linux_create_thread(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended);
platform_error	   linux_destroy_thread(platform_thread* thread);
platform_thread_id linux_this_thread_id();
platform_error	   linux_terminate_thread(platform_thread* thread, i32 exit_code);
void 	 		   linux_exit_this_thread(i32 exit_code);
void			   linux_thread_sleep(i32 ms);
i32  			   linux_get_num_cpus();
platform_thread_join_state linux_join_thread(platform_thread* thread, i32 ms); // ms = -1 for infinite

// may want to add 
	// WaitMultipleObjects
	// InterlockedCompareExchange

platform_error linux_create_semaphore(platform_semaphore* sem, i32 initial_count, i32 max_count);
platform_error linux_destroy_semaphore(platform_semaphore* sem);
platform_error linux_signal_semaphore(platform_semaphore* sem, i32 times); 
platform_semaphore_state linux_wait_semaphore(platform_semaphore* sem, i32 ms); // ms = -1 for infinite

void linux_create_mutex(platform_mutex* mut, bool aquire);
void linux_destroy_mutex(platform_mutex* mut);
void linux_try_aquire_mutex(platform_mutex* mut);
void linux_aquire_mutex(platform_mutex* mut);
void linux_release_mutex(platform_mutex* mut);

// allocates a string
string linux_make_timef(string fmt);
// takes a preallocated string, assumes enough space
void   linux_get_timef(string fmt, string* out);

