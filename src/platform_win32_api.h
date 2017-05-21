#pragma once

#include <windows.h>
#include "strings.h"

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
	DWORD error  = 0;		// something printable (for other platform layers)
};

struct platform_dll {
	// Opaque
	HMODULE dll_handle = {};
};

struct platform_file_attributes {
	// Opaque
	WIN32_FILE_ATTRIBUTE_DATA attrib = {};
};