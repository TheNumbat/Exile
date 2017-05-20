
#pragma once

#include <windows.h>
#include <gl/gl.h>
#include "wglext.h"
#include "platform_win32_api.h"

// API implementation prototypes

platform_error platform_create_window(platform_window* window, const char* title, u32 width, u32 height);
platform_error platform_destroy_window(platform_window* window);

platform_error platform_swap_buffers(platform_window* window);

bool 		   platform_process_messages(platform_window* window);
platform_error platform_wait_message();

platform_error platform_load_library(platform_dll* dll, const char* file_path);
platform_error platform_free_library(platform_dll* dll);

platform_api   platform_build_api();

platform_error platform_get_proc_address(void** address, platform_dll* dll, const char* name);

// Platform specific stuff

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext,
    												const int* attribList);
static wgl_create_context_attribs_arb* wglCreateContextAttribsARB;
static bool global_platform_running = true;

static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#include "platform_win32.cpp"
