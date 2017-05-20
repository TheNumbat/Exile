
#include "platform_win32.h"

platform_error platform_wait_message() {

	platform_error ret;

	if(WaitMessage() == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;
}

platform_error platform_swap_buffers(platform_window* window) {

	platform_error ret;

	if(SwapBuffers(window->device_context) == FALSE) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;
}

bool platform_process_messages(platform_window* window) {
	
	if(!global_platform_running) {
		return false;
	}

	MSG msg;

	while(PeekMessageA(&msg, window->handle, 0, 0, PM_REMOVE) != 0) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return true;
}

LRESULT CALLBACK window_proc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	if(msg == WM_DESTROY || msg == WM_CLOSE || msg == WM_QUIT) {
		
		global_platform_running = false;
		return 0;
	}

	return DefWindowProcA(handle, msg, wParam, lParam);
}

platform_error platform_create_window(platform_window* window, const char* title, u32 width, u32 height) {

	platform_error ret;

	window->title = title;
	window->width = width;
	window->height = height;

	HINSTANCE instance = GetModuleHandleA(NULL);

	if(instance == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	window->window_class = {
		sizeof(WNDCLASSEX),
		CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		(WNDPROC)window_proc,
		0, 0,
		instance,
		0, 0, 0, 0,
		window->title, 0
	};

	if(RegisterClassExA(&window->window_class) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	window->handle = CreateWindowExA(WS_EX_ACCEPTFILES, window->title, window->title, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			           				 CW_USEDEFAULT, CW_USEDEFAULT, window->width, window->height, 0, 0,
			           				 instance, 0);

	if(window->handle == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	window->device_context = GetDC(window->handle);

	if(window->device_context == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	window->pixel_format = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1, 
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 
		32, 8, 0, 8, 0, 8, 0, 8, 0,
		0, 0, 0, 0,
		24, 8, 0, 0, 0, 0, 0
	};

	int pixel_index = ChoosePixelFormat(window->device_context, &window->pixel_format);

	if(pixel_index == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	if(SetPixelFormat(window->device_context, pixel_index, 0) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	window->gl_temp = wglCreateContext(window->device_context);

	if(window->gl_temp == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	if(wglMakeCurrent(window->device_context, window->gl_temp) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	i32 attribs[9] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    	WGL_CONTEXT_MINOR_VERSION_ARB, 5,
    	WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    	0
	};

	wglCreateContextAttribsARB = (wgl_create_context_attribs_arb*)wglGetProcAddress("wglCreateContextAttribsARB");

	if(wglCreateContextAttribsARB == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	window->gl_context = wglCreateContextAttribsARB(window->device_context, 0, &attribs[0]);

	if(window->gl_context == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	if(wglMakeCurrent(window->device_context, window->gl_context) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	return ret;
}

platform_error platform_destroy_window(platform_window* window) {

	platform_error ret;

	if(wglDeleteContext(window->gl_temp) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}
	if(wglDeleteContext(window->gl_context) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}
	if(DestroyWindow(window->handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}
	return ret;
}