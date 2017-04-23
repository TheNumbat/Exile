
#import "strings.odin";
#import . "sys/windows.odin";

window :: struct {
	width, height:		i32,
	class:				WndClassExA,
	handle:				Hwnd,
	title:				string,
	device_context:		Hdc,
}

process_messages :: proc(w : window) -> bool {
	m : Msg;
	for PeekMessageA(^m, w.handle, 0, 0, PM_REMOVE) != 0 {
		TranslateMessage(^m);
		_ = DispatchMessageA(^m);
	}
	return true;
}

window_proc :: proc(handle : Hwnd, msg : u32, wparam : Wparam, lparam : Lparam) -> Lresult #cc_c {
	if msg == WM_DESTROY || msg == WM_CLOSE || msg == WM_QUIT {
		ExitProcess(0);
		return 0;
	}
	return DefWindowProcA(handle, msg, wparam, lparam);
}

make_window :: proc(t : string, w, h : i32) -> (window, i32) {

	win : window;

	win.width = w;
	win.height = h;
	win.title = t;

	instance := cast(Hinstance)GetModuleHandleA(nil);
	class_name := strings.new_c_string(win.title);
	defer free(class_name);

	win.class = WndClassExA{
		size 		= size_of(WndClassExA),
		style 		= CS_HREDRAW | CS_VREDRAW,
		wnd_proc 	= window_proc,
		instance	= instance,
		class_name	= class_name,
	};

	if RegisterClassExA(^win.class) == 0 {
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

	return win, 0;
}

destroy_window :: proc(w : window) -> i32 {
	if DestroyWindow(w.handle) != 0 {
		return 0;
	}
	return GetLastError();
}

