
#pragma once

platform_error platform_create_window(platform_window* window, string title, u32 width, u32 height);
platform_error platform_destroy_window(platform_window* window);

platform_error platform_swap_buffers(platform_window* window);

bool 		   platform_process_messages(platform_window* window);
platform_error platform_wait_message();

platform_error platform_load_library(platform_dll* dll, string file_path);
platform_error platform_free_library(platform_dll* dll);

platform_api   platform_build_api();

platform_error platform_get_proc_address(void** address, platform_dll* dll, string name);

platform_error platform_copy_file(string source, string dest, bool overwrite);
platform_error platform_get_file_attributes(platform_file_attributes* attrib, string file_path);
bool 		   platform_test_file_written(platform_file_attributes* first, platform_file_attributes* second);
platform_error platform_create_file(platform_file* file, string path, platform_file_open_op mode);
platform_error platform_close_file(platform_file* file);
platform_error platform_wrte_file(platform_file* file, void* mem, i32 bytes);
platform_error platform_get_stdout_as_file(platform_file* file);

// if this fails, we're having big problems
void*		   platform_heap_alloc(u64 bytes); // initialize memory to zero
void	  	   platform_heap_free(void* mem);

// allocates a string
platform_error platform_get_bin_path(string* path);

platform_error     platform_create_thread(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended);
platform_error	   platform_destroy_thread(platform_thread* thread);
platform_thread_id platform_this_thread_id();
platform_error	   platform_terminate_thread(platform_thread* thread, i32 exit_code);
void 	 		   platform_exit_this_thread(i32 exit_code);
void			   platform_thread_sleep(i32 ms);
i32  			   platform_get_num_cpus();
platform_thread_join_state platform_join_thread(platform_thread* thread, i32 ms); // ms = -1 for infinite

// may want to add 
	// WaitMultipleObjects
	// InterlockedCompareExchange

platform_error platform_create_semaphore(platform_semaphore* sem, i32 initial_count, i32 max_count);
platform_error platform_destroy_semaphore(platform_semaphore* sem);
platform_error platform_signal_semaphore(platform_semaphore* sem, i32 times); 
platform_semaphore_state platform_wait_semaphore(platform_semaphore* sem, i32 ms); // ms = -1 for infinite

platform_error platform_create_mutex(platform_mutex* mut, bool aquire);
platform_error platform_destroy_mutex(platform_mutex* mut);
platform_mutex_state platform_aquire_mutex(platform_mutex* mut, i32 ms); // ms = -1 for infinite
platform_error platform_release_mutex(platform_mutex* mut);

#ifdef _WIN32

#include "platform_win32.h"

#else

#error "Unsupported platform."

#endif

