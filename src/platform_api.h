#pragma once

#ifdef _WIN32

#include "platform_win32_api.h"

#else

#error "Unsupported platform."

#endif

struct platform_api {
	platform_error (*platform_create_window)(platform_window* window, const char* title, u32 width, u32 height)	= NULL;
	platform_error (*platform_destroy_window)(platform_window* window)											= NULL;
	platform_error (*platform_swap_buffers)(platform_window* window)											= NULL;
	bool 		   (*platform_process_messages)(platform_window* window)										= NULL;
	platform_error (*platform_wait_message)()																	= NULL;
	platform_error (*platform_load_library)(platform_dll* dll, const char* file_path)							= NULL;
	platform_error (*platform_free_library)(platform_dll* dll)													= NULL;
	platform_error (*platform_get_proc_address)(void** address, platform_dll* dll, const char* name)			= NULL;
};

