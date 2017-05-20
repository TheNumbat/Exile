
#pragma once

#include <windows.h>
#include <gl\gl.h>
#include "wglext.h"

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

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext,
    												const int* attribList);

static wgl_create_context_attribs_arb* wglCreateContextAttribsARB;
static bool global_platform_running = true;

platform_error platform_create_window(platform_window* window, const char* title, u32 width, u32 height);
platform_error platform_destroy_window(platform_window* window);

platform_error platform_swap_buffers(platform_window* window);

bool platform_process_messages(platform_window* window);
platform_error platform_wait_message();

static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#include "platform_win32.cpp"
