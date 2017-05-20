#pragma once

#include <windows.h>

// API Structures (mark opaque/transparent)

struct platform_window {
	const char* title	= "";
	u32 width, height	= 0;

	// Opaque block, do not use outside of platform code
	WNDCLASSEX window_class				= {};
	HWND handle							= {};
	HDC device_context					= {};
	PIXELFORMATDESCRIPTOR pixel_format 	= {};
	HGLRC gl_temp, gl_context		   	= {};
};

struct platform_error {
	bool good	 = true;
	DWORD error  = 0;		// something printable (for other platform layers)
};

struct platform_dll {
	// Opaque
	HMODULE dll_handle;
};

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
