#pragma once

#include "common.h"
#include "strings.h"

struct platform_window;
struct platform_error;
struct platform_dll;
struct platform_file_attributes;

struct platform_api {
	platform_error (*platform_create_window)(platform_window* window, string title, u32 width, u32 height)		= NULL;
	platform_error (*platform_destroy_window)(platform_window* window)												= NULL;
	platform_error (*platform_swap_buffers)(platform_window* window)												= NULL;
	bool 		   (*platform_process_messages)(platform_window* window)											= NULL;
	platform_error (*platform_wait_message)()																		= NULL;
	platform_error (*platform_load_library)(platform_dll* dll, string file_path)								= NULL;
	platform_error (*platform_free_library)(platform_dll* dll)														= NULL;
	platform_error (*platform_get_proc_address)(void** address, platform_dll* dll, string name)				= NULL;
	platform_error (*platform_get_file_attributes)(platform_file_attributes* attrib, string file_path)			= NULL;
	bool 		   (*platform_test_file_written)(platform_file_attributes* first, platform_file_attributes* second) = NULL;
	platform_error (*platform_copy_file)(string source, string dest, bool overwrite)						= NULL;
	void*		   (*platform_heap_alloc)(u64 bytes)																= NULL;
	void	  	   (*platform_heap_free)(void* mem)																	= NULL;
	platform_error (*platform_get_bin_path)(string* path) /* heap_allocs a string */									= NULL;
};

#ifdef _WIN32

#include "platform_win32_api.h"

#else

#error "Unsupported platform."

#endif
