#pragma once

#include "windows.h"
#ifdef _MSC_VER
#include <intrin.h>
#else
#error __cpuid
#endif

// API Structures (mark opaque/transparent)

u32 PLATFORM_SHARING_ERROR = 32;

struct platform_window {
	// Transparent
	string title;
	i32 w = 0, h = 0;

	// Opaque block, do not use outside of platform code
	WNDCLASSEXA window_class			= {};
	HWND handle							= {};
	HDC device_context					= {};
	HGLRC gl_context					= {};
};

struct platform_dll {
	// Opaque
	HMODULE dll_handle = {};
};

struct platform_file_attributes {
	// Opaque
	WIN32_FILE_ATTRIBUTE_DATA attrib = {};
};

struct platform_thread {
	// transparent
	platform_thread_id id;

	// Opaque
	HANDLE handle 	= null;
};

struct platform_semaphore {
	// Opaque
	HANDLE handle 	= null;
};

struct platform_mutex {
	// Opaque
	CRITICAL_SECTION cs = {};
};

struct platform_file {
	// Transparent
	string path;

	// Opaque
	HANDLE handle 	= null;
};

bool operator==(platform_file first, platform_file second) {
	return first.handle == second.handle;
}
