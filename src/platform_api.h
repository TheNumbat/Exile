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
	platform_error (*platform_create_window)(platform_window* window, string title, u32 width, u32 height)					 = NULL;
	platform_error (*platform_destroy_window)(platform_window* window)														 = NULL;
	platform_error (*platform_swap_buffers)(platform_window* window)														 = NULL;
	bool 		   (*platform_process_messages)(platform_window* window)													 = NULL;
	platform_error (*platform_wait_message)()																				 = NULL;
	platform_error (*platform_load_library)(platform_dll* dll, string file_path)											 = NULL;
	platform_error (*platform_free_library)(platform_dll* dll)																 = NULL;
	platform_error (*platform_get_proc_address)(void** address, platform_dll* dll, string name)								 = NULL;
	platform_error (*platform_get_file_attributes)(platform_file_attributes* attrib, string file_path)						 = NULL;
	bool 		   (*platform_test_file_written)(platform_file_attributes* first, platform_file_attributes* second) 		 = NULL;
	platform_error (*platform_copy_file)(string source, string dest, bool overwrite)										 = NULL;
	void*		   (*platform_heap_alloc)(u64 bytes)																		 = NULL;
	void	  	   (*platform_heap_free)(void* mem)																			 = NULL;
	platform_error (*platform_get_bin_path)(string* path) /* heap_allocs a string */										 = NULL;
	platform_error (*platform_create_thread)(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended) = NULL;
	platform_thread_id (*platform_this_thread_id)()																			 = NULL;
	platform_error (*platform_terminate_thread)(platform_thread* thread, i32 exit_code)										 = NULL;
	void 		   (*platform_exit_this_thread)(i32 exit_code)																 = NULL;
	void		   (*platform_thread_sleep)(i32 ms)																			 = NULL;
	platform_error (*platform_destroy_thread)(platform_thread* thread)														 = NULL;
	platform_error (*platform_create_semaphore)(platform_semaphore* sem, i32 initial_count, i32 max_count)					 = NULL;
	platform_error (*platform_destroy_semaphore)(platform_semaphore* sem)													 = NULL;
	platform_error (*platform_signal_semaphore)(platform_semaphore* sem, i32 times)											 = NULL;
	platform_semaphore_state (*platform_wait_semaphore)(platform_semaphore* sem, i32 ms)									 = NULL;
	platform_error (*platform_create_mutex)(platform_mutex* mut, bool aquire)												 = NULL;
	platform_error (*platform_destroy_mutex)(platform_mutex* mut)															 = NULL;
	platform_mutex_state (*platform_aquire_mutex)(platform_mutex* mut, i32 ms)												 = NULL;
	platform_error (*platform_release_mutex)(platform_mutex* mut)															 = NULL;
	i32   		   (*platform_get_num_cpus)()																				 = NULL;
	platform_thread_join_state (*platform_join_thread)(platform_thread* thread, i32 ms)										 = NULL;
	platform_error (*platform_create_file)(platform_file* file, string path, platform_file_open_op mode)					 = NULL;
	platform_error (*platform_close_file)(platform_file* file)																 = NULL;
	platform_error (*platform_write_file)(platform_file* file, void* mem, i32 bytes)										 = NULL;
	platform_error (*platform_get_stdout_as_file)(platform_file* file)														 = NULL;
	string (*platform_make_timef)(string fmt)																				 = NULL;
	void (*platform_get_timef)(string fmt, string* out)																		 = NULL;
};

#ifdef _WIN32

#include "platform_win32_api.h"

#else

#error "Unsupported platform."

#endif
