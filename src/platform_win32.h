
#pragma once

#include <windows.h>
#include <gl/gl.h>
#include "platform_win32_api.h"

// Platform specific stuff

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext,
													const int* attribList);

static wgl_create_context_attribs_arb* wglCreateContextAttribsARB;
static bool global_platform_running = true;

static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#include "platform_win32.cpp"
