#pragma once

#include "windows.h"
#ifdef _MSC_VER
#include <intrin.h>
#else
#error __cpuid
#endif

#define WIN32_SHARING_ERROR 32

enum class wgl_context : int {
	major_version_arb     		= 0x2091,
	minor_version_arb     		= 0x2092,
	flags_arb             		= 0x2094,
	forward_compatible_bit_arb 	= 0x0002,
	profile_mask_arb      		= 0x9126,
	core_profile_bit_arb  		= 0x0001,
	sample_buffers_arb          = 0x2041,
	samples_arb                 = 0x2042,
	draw_to_window_arb          = 0x2001,
	support_opengl_arb          = 0x2010,
	double_buffer_arb           = 0x2011,
	pixel_type_arb              = 0x2013,
	color_bits_arb              = 0x2014,
	depth_bits_arb              = 0x2022,
	stencil_bits_arb            = 0x2023,
	alpha_bits_arb              = 0x201B,
	type_rgba_arb               = 0x202B
};

struct platform_window_internal {
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
