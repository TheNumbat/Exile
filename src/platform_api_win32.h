#pragma once

// #define USE_DEFAULT_HEADERS

#ifdef USE_DEFAULT_HEADERS
#define NOMINMAX           		1
#define WIN32_LEAN_AND_MEAN		1
#define WIN32_MEAN_AND_LEAN		1
#define VC_EXTRALEAN       		1
#include <windows.h>
#include <gl/gl.h>
#include <windowsx.h>
#include "../../deps/glext.h"
#include "../../deps/wglext.h"
#else
#include "platform/windows.h"
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
	PIXELFORMATDESCRIPTOR pixel_format 	= {};
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

struct platform_thread_id {
	// Transparent
	DWORD id 	= null;
};

bool operator==(platform_thread_id first, platform_thread_id second) {
	return first.id == second.id;
}

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
