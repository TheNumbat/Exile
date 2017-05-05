
#import "strings.odin";
#import . "windows.odin";
#import . "sys/windows.odin";
#import wgl "sys/wgl.odin";

global_running := true;

window :: struct {
	width, height:		i32,
	class:				WndClassExA,
	handle:				Hwnd,
	title:				string,
	device_context:		Hdc,
	pixel_format:		PIXELFORMATDESCRIPTOR,
	gl_temp,
	gl_context:			wgl.Hglrc,
}

swap_window :: proc(w : window) {
	SwapBuffers(w.device_context);
}

wait :: proc() {
	WaitMessage();
}

process_messages :: proc(w : window) -> bool {
	if(!global_running) {
		return false;
	}
	m : Msg;
	for PeekMessageA(&m, w.handle, 0, 0, PM_REMOVE) != 0 {
		TranslateMessage(&m);
		_ = DispatchMessageA(&m);
	}
	return true;
}

window_proc :: proc(handle : Hwnd, msg : u32, wparam : Wparam, lparam : Lparam) -> Lresult #cc_c {
	if msg == WM_DESTROY || msg == WM_CLOSE || msg == WM_QUIT {
		global_running = false; // this was being strange
		return 0;
	}
	return DefWindowProcA(handle, msg, wparam, lparam);
}

make_window :: proc(t : string, w, h : i32) -> (window, i32) {

	win : window;

	win.width = w;
	win.height = h;
	win.title = t;

	instance := Hinstance(GetModuleHandleA(nil));
	class_name := strings.new_c_string(win.title);
	defer free(class_name);

	win.class = WndClassExA{
		size 		= size_of(WndClassExA),
		style 		= CS_HREDRAW | CS_VREDRAW,
		wnd_proc 	= window_proc,
		instance	= instance,
		class_name	= class_name,
	};

	if RegisterClassExA(&win.class) == 0 {
		return win, GetLastError();
	}

	win.handle = CreateWindowExA(0, class_name, class_name, 
								 WS_VISIBLE | WS_OVERLAPPEDWINDOW, 
								 CW_USEDEFAULT, CW_USEDEFAULT,
								 w, h, nil, nil, instance, nil);

	if win.handle == nil {
		return win, GetLastError();
	}

	win.device_context = GetDC(win.handle);

	if win.device_context == nil {
		return win, GetLastError();
	}

	win.pixel_format = PIXELFORMATDESCRIPTOR{
		size 			= size_of(PIXELFORMATDESCRIPTOR),
		version 		= 1,
		flags			= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		pixel_type		= PFD_TYPE_RGBA,
		color_bits		= 32,
		alpha_bits		= 8,
		depth_bits		= 24,
		stencil_bits	= 8,
		layer_type 		= PFD_MAIN_PLANE,
	};

	format_index := ChoosePixelFormat(win.device_context, &win.pixel_format);
	if format_index == 0 {
		return win, GetLastError();
	}

	if SetPixelFormat(win.device_context, format_index, nil) == 0 {
		return win, GetLastError();
	}

	win.gl_temp = wgl.CreateContext(win.device_context);

	if win.gl_temp == nil {
		return win, GetLastError();
	}

	if wgl.MakeCurrent(win.device_context, win.gl_temp) == 0 {
		return win, GetLastError();
	}

	attribs := [9]i32{
		wgl.CONTEXT_MAJOR_VERSION_ARB, 3,
		wgl.CONTEXT_MINOR_VERSION_ARB, 3,
		wgl.CONTEXT_PROFILE_MASK_ARB, wgl.CONTEXT_CORE_PROFILE_BIT_ARB,
		wgl.CONTEXT_FLAGS_ARB, wgl.CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0
	};

	modern_context_func := "wglCreateContextAttribsARB";
	c_modern_context_func := strings.new_c_string(modern_context_func);
	defer free(c_modern_context_func);

	wglCreateContextAttribsARB := wgl.CreateContextAttribsARB_Type(
										wgl.GetProcAddress(c_modern_context_func));

	win.gl_context = wglCreateContextAttribsARB(win.device_context, nil, &attribs[0]);

	if win.gl_context == nil {
		return win, GetLastError();
	}

	if wgl.MakeCurrent(win.device_context, win.gl_context) == 0 {
		return win, GetLastError();
	}

	return win, 0;
}

destroy_window :: proc(w : window) -> i32 {
	if wgl.DeleteContext(w.gl_temp) == 0 {
		return GetLastError();
	}
	if wgl.DeleteContext(w.gl_context) == 0 {
		return GetLastError();
	}
	if DestroyWindow(w.handle) == 0 {
		return GetLastError();
	}
	return 0;
}

