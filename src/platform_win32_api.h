#pragma once

#include <windows.h>
#include <gl/gl.h>

// API Structures (mark opaque/transparent)

struct platform_window {
	// Transparent
	string title;
	u32 width, height	= 0;

	// Opaque block, do not use outside of platform code
	WNDCLASSEX window_class				= {};
	HWND handle							= {};
	HDC device_context					= {};
	PIXELFORMATDESCRIPTOR pixel_format 	= {};
	HGLRC gl_temp, gl_context		   	= {};
};

struct platform_error {
	// Transparent
	bool good	 = true;
	// Transparent - printf %i able
	DWORD error  = 0;		
};

struct platform_dll {
	// Opaque
	HMODULE dll_handle = {};
};

struct platform_file_attributes {
	// Opaque
	WIN32_FILE_ATTRIBUTE_DATA attrib = {};
};

struct platform_thread_id {
	// Transparent - printf %i able
	DWORD id 	= NULL;
};

bool operator==(platform_thread_id& first, platform_thread_id& second) {
	return first.id == second.id;
}

struct platform_thread {
	// transparent
	platform_thread_id id;

	// Opaque
	HANDLE handle 	= NULL;
};

struct platform_semaphore {
	// Opaque
	HANDLE handle 	= NULL;
};

struct platform_mutex {
	// Opaque
	HANDLE handle 	= NULL;
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

struct platform_file {
	// Transparent
	string path;

	// Opaque
	HANDLE handle 	= NULL;
};
