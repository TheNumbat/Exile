
#include "platform_win32.h"

#ifdef TEST_NET_ZERO_ALLOCS
#define win32_heap_alloc win32_heap_alloc_net
#define win32_heap_free win32_heap_free_net
#endif

extern "C" {
	// Request dGPU
	__declspec(dllexport) bool NvOptimusEnablement = true;
	__declspec(dllexport) bool AmdPowerXpressRequestHighPerformance = true;
}

wglCreateContextAttribsARB_t	wglCreateContextAttribsARB;
wglChoosePixelFormatARB_t 		wglChoosePixelFormatARB;
wglSwapIntervalEXT_t 			wglSwapIntervalEXT;

void (*global_enqueue)(void* queue_param, platform_event evt) = null;
void* global_enqueue_param = null;

POINT saved_mouse_pos = {};
string clipboard;

void* win32_heap_alloc_net(u64 bytes);
void win32_heap_free_net(void* mem);

void platform_test_api() {}
void platform_shutdown() {

	if(clipboard.c_str) free_string(clipboard, win32_heap_free);
}

platform_api platform_build_api() {

	platform_api ret = {};

	ret.get_perfcount			= &win32_get_perfcount;
	ret.get_perfcount_freq		= &win32_get_perfcount_freq;
	ret.create_window			= &win32_create_window;
	ret.destroy_window			= &win32_destroy_window;
	ret.swap_buffers			= &win32_swap_buffers;
	ret.set_queue_callback		= &win32_set_queue_callback;
	ret.pump_events				= &win32_pump_events;
	ret.queue_event 			= &win32_queue_event;
	ret.wait_message			= &win32_wait_message;
	ret.load_library			= &win32_load_library;
	ret.free_library			= &win32_free_library;
	ret.get_proc_address 		= &win32_get_proc_address;
	ret.get_file_attributes 	= &win32_get_file_attributes;
	ret.test_file_written		= &win32_test_file_written;
	ret.copy_file				= &win32_copy_file;
	ret.heap_alloc				= &win32_heap_alloc;
	ret.heap_free				= &win32_heap_free;
	ret.heap_realloc			= &win32_heap_realloc;
	ret.get_bin_path			= &win32_get_bin_path;
	ret.create_thread			= &win32_create_thread;
	ret.this_thread_id			= &win32_this_thread_id;
	ret.thread_sleep			= &win32_thread_sleep;
	ret.create_semaphore		= &win32_create_semaphore;
	ret.destroy_semaphore		= &win32_destroy_semaphore;
	ret.signal_semaphore		= &win32_signal_semaphore;
	ret.wait_semaphore			= &win32_wait_semaphore;
	ret.create_mutex			= &win32_create_mutex;
	ret.destroy_mutex			= &win32_destroy_mutex;
	ret.aquire_mutex			= &win32_aquire_mutex;
	ret.try_aquire_mutex		= &win32_try_aquire_mutex;
	ret.release_mutex			= &win32_release_mutex;
	ret.destroy_thread			= &win32_destroy_thread;
	ret.get_num_cpus			= &win32_get_num_cpus;
	ret.join_thread				= &win32_join_thread;
	ret.create_file				= &win32_create_file;
	ret.close_file				= &win32_close_file;
	ret.write_file				= &win32_write_file;
	ret.read_file				= &win32_read_file;
	ret.time_string				= &win32_time_string;
	ret.get_window_size			= &win32_get_window_size;
	ret.write_stdout			= &win32_write_stdout;
	ret.write_stdout_str		= &win32_write_stdout_str;
	ret.file_size				= &win32_file_size;
	ret.get_glproc				= &win32_get_glproc;
	ret.keydown					= &win32_keydown;
	ret.is_debugging			= &win32_is_debugging;
	ret.debug_break				= &win32_debug_break;
	ret.set_cursor				= &win32_set_cursor;
	ret.capture_mouse			= &win32_capture_mouse;
	ret.release_mouse			= &win32_release_mouse;
	ret.set_cursor_pos			= &win32_set_cursor_pos;
	ret.window_focused			= &win32_window_focused;
	ret.atomic_exchange 		= &win32_atomic_exchange;
	ret.get_phys_cpus 			= &win32_get_phys_cpus;
	ret.get_cursor_pos 			= &win32_get_cursor_pos;
	ret.mousedown 				= &win32_mousedown;
	ret.show_cursor 			= &win32_show_cursor;
	ret.get_scancode 			= &win32_get_scancode;
	ret.get_window_drawable 	= &win32_get_window_drawable;
	ret.cursor_shown 			= &win32_cursor_shown;
	ret.get_clipboard 			= &win32_get_clipboard;
	ret.set_clipboard 			= &win32_set_clipboard;
	ret.recreate_window			= &win32_recreate_window;
	ret.apply_window_settings 	= &win32_apply_window_settings;

	return ret;
}

bool win32_apply_window_settings(platform_window* window) {

	return true;
}

string win32_get_clipboard() {

	string nothing;

	if(!OpenClipboard(null)) return nothing;

	HANDLE wide_buf = GetClipboardData(CF_UNICODETEXT);
	if(!wide_buf) {
		CloseClipboard();
		return nothing;
	}

	if(wchar_t* g_wide_buf = (wchar_t*)GlobalLock(wide_buf)) {

		i32 len = WideCharToMultiByte(CP_UTF8, 0, g_wide_buf, -1, null, 0, null, null);

		if((u32)len > clipboard.cap) {
			if(clipboard.c_str) free_string(clipboard, win32_heap_free);
			clipboard = make_string(len, win32_heap_alloc);
		}

		WideCharToMultiByte(CP_UTF8, 0, g_wide_buf, -1, clipboard.c_str, clipboard.cap, null, null);
	}

	GlobalUnlock(wide_buf);
	CloseClipboard();
	return clipboard;
}

void win32_set_clipboard(string text) {

	if(!OpenClipboard(null)) return;

	i32 len = MultiByteToWideChar(CP_UTF8, 0, text.c_str, text.cap, null, 0);

	HGLOBAL wide_buf = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)len * sizeof(wchar_t));
	if(!wide_buf) {
		CloseClipboard();
		return;
	}

	wchar_t* g_wide_buf = (wchar_t*)GlobalLock(wide_buf);

	MultiByteToWideChar(CP_UTF8, 0, text.c_str, text.cap, g_wide_buf, len);

	GlobalUnlock(wide_buf);
	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, wide_buf);
	CloseClipboard();
}

bool win32_cursor_shown() {
	
	CURSORINFO info = {};
	info.cbSize = sizeof(CURSORINFO);

	GetCursorInfo(&info);

	return info.flags & CURSOR_SHOWING;
}

bool win32_mousedown(platform_mouseflag button) {

	switch(button) {
	case platform_mouseflag::lclick: {
		return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	} break;
	case platform_mouseflag::rclick: {
		return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	} break;
	case platform_mouseflag::mclick: {
		return (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
	} break;
	case platform_mouseflag::x1click: {
		return (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
	} break;
	case platform_mouseflag::x2click: {
		return (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
	} break;
	}

	return false;
}

platform_error win32_get_cursor_pos(platform_window* win, i32* x, i32* y) {

	platform_error ret;

	POINT p;

	if(!GetCursorPos(&p)) {
		ret.good = false;
		ret.error = GetLastError();
	}

	if(!ScreenToClient(win->handle, &p)) {
		ret.good = false;
		ret.error = GetLastError();
	}

	*x = p.x;
	*y = p.y;

	return ret;
}

i32 win32_get_phys_cpus() {
	
	i32 cpus = win32_get_num_cpus();

	i32 cpuinfo[4];
	__cpuid(cpuinfo, 1);
	
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	
	return HT ? cpus / 2 : cpus;
}

u64 win32_atomic_exchange(u64* dest, u64 val) {

	return _InterlockedExchange64((LONGLONG volatile*)dest, val);
}

bool win32_window_focused(platform_window* win) {

	return win->handle == GetFocus();
}

platform_error win32_set_cursor_pos(platform_window* win, i32 x, i32 y) {

	platform_error ret;

	POINT p = {x,y};
	if(!ClientToScreen(win->handle, &p)) {
		ret.good = false;
		ret.error = GetLastError();
	}

	if(!SetCursorPos(p.x, p.y)) {
		ret.good = false;
		ret.error = GetLastError();
	}

	return ret;
}

void win32_show_cursor(bool show) {

	ShowCursor(show);
}

void win32_capture_mouse(platform_window* win) {

	GetCursorPos(&saved_mouse_pos);
	ShowCursor(FALSE);
	SetCapture(win->handle);

	RAWINPUTDEVICE raw_mouse = {0x01, 0x02, RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE, win->handle};
	RegisterRawInputDevices(&raw_mouse, 1, sizeof(RAWINPUTDEVICE));
}

void win32_release_mouse(platform_window* win) {

	ReleaseCapture();
	ShowCursor(TRUE);
	SetCursorPos(saved_mouse_pos.x, saved_mouse_pos.y);

	RAWINPUTDEVICE raw_mouse = {0x01, 0x02, RIDEV_REMOVE, null};
	RegisterRawInputDevices(&raw_mouse, 1, sizeof(RAWINPUTDEVICE));
}

u64 win32_get_perfcount() {

	u64 ret;
	QueryPerformanceCounter((LARGE_INTEGER*)&ret);
	return ret;
}

u64 win32_get_perfcount_freq() {

	u64 ret;
	QueryPerformanceFrequency((LARGE_INTEGER*)&ret);
	return ret;
}

void win32_debug_break() {
	__debugbreak();	
}

bool win32_is_debugging() {
	return IsDebuggerPresent() == TRUE;
}

void win32_set_cursor(platform_window* window, platform_cursor c) {

	i32 x, y, w, h;
	win32_get_cursor_pos(window, &x, &y);
	win32_get_window_drawable(window, &w, &h);
	if(x < 0 || y < 0 || x > w || y > h) return;

	switch(c) {
	case platform_cursor::pointer: {
		SetCursor(LoadCursorA(0, IDC_ARROW));
		break;
	}
	case platform_cursor::crosshair: {
		SetCursor(LoadCursorA(0, IDC_CROSS));
		break;
	}
	case platform_cursor::hand: {
		SetCursor(LoadCursorA(0, IDC_HAND));
		break;
	}
	case platform_cursor::help: {
		SetCursor(LoadCursorA(0, IDC_HELP));
		break;
	}
	case platform_cursor::I: {
		SetCursor(LoadCursorA(0, IDC_IBEAM));
		break;
	}
	case platform_cursor::hourglass: {
		SetCursor(LoadCursorA(0, IDC_WAIT));
		break;
	}
	}
}

platform_error win32_write_stdout(void* mem, u32 len) {

	platform_error ret;

	if(WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), mem, len, 0, 0) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	return ret;	
}

platform_error win32_write_stdout_str(string str) {

	return win32_write_stdout(str.c_str, str.len - 1);
}

platform_error win32_get_window_drawable(platform_window* window, i32* w, i32* h) {

	platform_error ret;

	RECT rect;

	if(GetClientRect(window->handle, &rect) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	*w = rect.right - rect.left;
	*h = rect.bottom - rect.top;

	return ret;
}

platform_error win32_get_window_size(platform_window* window, i32* w, i32* h) {

	platform_error ret;

	RECT rect;

	if(GetWindowRect(window->handle, &rect) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	*w = rect.right - rect.left;
	*h = rect.bottom - rect.top;

	return ret;
}

string win32_time_string() {

	i32 len = GetTimeFormatA(LOCALE_USER_DEFAULT, 0, null, "hh:mm:ss", null, 0);

	string ret = make_string(len, &win32_heap_alloc);
	ret.len = ret.cap;

	GetTimeFormatA(LOCALE_USER_DEFAULT, 0, null, "hh:mm:ss", ret.c_str, len);	

	return ret;
}

u32 win32_file_size(platform_file* file) {
	return GetFileSize(file->handle, null);
}

platform_error win32_read_file(platform_file* file, void* mem, u32 bytes) {

	platform_error ret;

	if(ReadFile(file->handle, mem, bytes, null, null) == FALSE) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	return ret;
}

platform_error win32_write_file(platform_file* file, void* mem, u32 bytes) {

	platform_error ret;

	if(WriteFile(file->handle, mem, (DWORD)bytes, null, null) == FALSE) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;
}

platform_error win32_create_file(platform_file* file, string path, platform_file_open_op mode) {

	platform_error ret;

	DWORD creation = 0;
	switch(mode) {
		case platform_file_open_op::existing:
		creation = OPEN_EXISTING;
		break;
		case platform_file_open_op::cleared:
		creation = CREATE_ALWAYS;
		break;
		default:
		ret.good = false;
		return ret;	
		break;
	}

	file->handle = CreateFileA(path.c_str, GENERIC_READ | GENERIC_WRITE, 0, null, creation, FILE_ATTRIBUTE_NORMAL, null);

	if(file->handle == INVALID_HANDLE_VALUE) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	file->path = path;

	return ret;
}

platform_error win32_close_file(platform_file* file) {

	platform_error ret;

	if(CloseHandle(file->handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	file->handle = null;

	return ret;
}

platform_thread_join_state win32_join_thread(platform_thread* thread, i32 ms) {
	
	platform_thread_join_state ret;

	switch(WaitForSingleObject(thread->handle, ms == -1 ? INFINITE : (DWORD)ms)) {
	case WAIT_OBJECT_0:
		ret.state = _platform_thread_join_state::joined;
		return ret;
	case WAIT_TIMEOUT:
		ret.state = _platform_thread_join_state::timed_out;
		return ret;
	case WAIT_FAILED:
		ret.state = _platform_thread_join_state::failed;
		ret.error.good = false;
		ret.error.error = GetLastError();
		return ret;		
	}

	return ret;
}

i32 win32_get_num_cpus() {

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
}

platform_error win32_create_semaphore(platform_semaphore* sem, i32 initial_count, i32 max_count) {

	platform_error ret;

	sem->handle = CreateSemaphoreExA(null, initial_count, max_count, null, 0, SEMAPHORE_ALL_ACCESS);

	if(sem->handle == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

platform_error win32_destroy_semaphore(platform_semaphore* sem) {
	
	platform_error ret;

	if(CloseHandle(sem->handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	sem->handle = null;

	return ret;
}

platform_error win32_signal_semaphore(platform_semaphore* sem, i32 times) {

	platform_error ret;

	if(ReleaseSemaphore(sem->handle, (LONG)times, null) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;
}

platform_semaphore_state win32_wait_semaphore(platform_semaphore* sem, i32 ms) {

	platform_semaphore_state ret;

	switch(WaitForSingleObject(sem->handle, ms == -1 ? INFINITE : (DWORD)ms)) {
	case WAIT_OBJECT_0:
		ret.state = _platform_semaphore_state::signaled;
		return ret;
	case WAIT_TIMEOUT:
		ret.state = _platform_semaphore_state::timed_out;
		return ret;
	case WAIT_FAILED:
		ret.state = _platform_semaphore_state::failed;
		ret.error.good = false;
		ret.error.error = GetLastError();
		return ret;		
	}

	return ret;
}

void win32_create_mutex(platform_mutex* mut, bool aquire) {

	InitializeCriticalSection(&mut->cs);

	if(aquire) {
		EnterCriticalSection(&mut->cs);
	}
}

void win32_destroy_mutex(platform_mutex* mut) {
	
	DeleteCriticalSection(&mut->cs);
	mut->cs = {};
}

void win32_aquire_mutex(platform_mutex* mut) {

	EnterCriticalSection(&mut->cs);
}

bool win32_try_aquire_mutex(platform_mutex* mut) {

	return TryEnterCriticalSection(&mut->cs) == TRUE;
}

void win32_release_mutex(platform_mutex* mut) {

	LeaveCriticalSection(&mut->cs);
}

platform_error win32_destroy_thread(platform_thread* thread) {

	platform_error ret;

	if(CloseHandle(thread->handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	thread->handle = null;

	return ret;
}

void win32_thread_sleep(i32 ms) {
	Sleep(ms);
}

platform_thread_id win32_this_thread_id() {

	platform_thread_id ret;

	ret = GetCurrentThreadId();

	return ret;
}

platform_error win32_create_thread(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended) {

	platform_error ret;

	thread->handle = CreateThread(null, 0, (LPTHREAD_START_ROUTINE)proc, param, start_suspended ? CREATE_SUSPENDED : 0, (DWORD*)&thread->id);

	if(thread->handle == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;			
	}

	return ret;
}

platform_error win32_get_bin_path(string* path) {

	platform_error ret;

	*path = make_string(MAX_PATH, &win32_heap_alloc);
	
	DWORD len = GetModuleFileNameA(null, (LPSTR)path->c_str, MAX_PATH);

	if(len == 0) {
		ret.good = false;
		ret.error = GetLastError();

		free_string(*path, &win32_heap_free);

		return ret;
	}

	if(path->c_str[len - 1] == '\0') {
		ret.good = false;
		ret.error = GetLastError();

		free_string(*path, &win32_heap_free);

		return ret;
	}

	path->len = len;

	return ret;
}

#ifdef TEST_NET_ZERO_ALLOCS
void* win32_heap_alloc_net(u64 bytes) {

	HANDLE heap = GetProcessHeap();
	void* ret = HeapAlloc(heap, HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, (SIZE_T)bytes);

	_InterlockedIncrement((LONG volatile*)&global_num_allocs);

	return ret;
}

void win32_heap_free_net(void* mem) {

	HANDLE heap = GetProcessHeap();
	HeapFree(heap, 0, mem);

	_InterlockedDecrement((LONG volatile*)&global_num_allocs);
}
#else
void* win32_heap_alloc(u64 bytes) {

	HANDLE heap = GetProcessHeap();
	void* ret = HeapAlloc(heap, HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, (SIZE_T)bytes);

	return ret;
}

void win32_heap_free(void* mem) {

	HANDLE heap = GetProcessHeap();
	HeapFree(heap, 0, mem);
}
#endif

void* win32_heap_realloc(void* mem, u64 bytes) {

	HANDLE heap = GetProcessHeap();
	void* ret = HeapReAlloc(heap, HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, mem, (SIZE_T)bytes);

	return ret;
}

platform_error win32_copy_file(string source, string dest, bool overwrite) {

	platform_error ret;

	if(CopyFileA(source.c_str, dest.c_str, !overwrite) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;			
	}

	return ret;
}

bool win32_test_file_written(platform_file_attributes* first, platform_file_attributes* second) {

	return CompareFileTime(&first->attrib.ftLastWriteTime, &second->attrib.ftLastWriteTime) == -1;
}

platform_error win32_get_file_attributes(platform_file_attributes* attrib, string file_path) {

	platform_error ret;

	if(GetFileAttributesExA(file_path.c_str, GetFileExInfoStandard, (LPVOID)&attrib->attrib) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

void* win32_get_glproc(string name) {
	return wglGetProcAddress(name.c_str);
}

platform_error win32_get_proc_address(void** address, platform_dll* dll, string name) {

	platform_error ret;

	*address = GetProcAddress(dll->dll_handle, name.c_str);

	if(*address == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

platform_error win32_load_library(platform_dll* dll, string file_path) {

	platform_error ret;

	dll->dll_handle = LoadLibraryA(file_path.c_str);

	if(dll->dll_handle == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

platform_error win32_free_library(platform_dll* dll) {
	
	platform_error ret;

	if(FreeLibrary(dll->dll_handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

platform_error win32_wait_message() {

	platform_error ret;

	if(WaitMessage() == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;
}

platform_error win32_swap_buffers(platform_window* window) {

	platform_error ret;

	if(SwapBuffers(window->device_context) == FALSE) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;
}

// we may need to enqueue events both here and in window_proc

void win32_set_queue_callback(void (*enqueue)(void* queue_param, platform_event evt), void* queue_param) {

	global_enqueue = enqueue;
	global_enqueue_param = queue_param;
}

void win32_pump_events(platform_window* window) {
	
	MSG msg;

	while(PeekMessageA(&msg, window->handle, 0, 0, PM_REMOVE) != 0) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void win32_queue_event(platform_event evt) {

	global_enqueue(global_enqueue_param, evt);
}

u16 translate_key_code(platform_keycode keycode) {
	switch(keycode) {
	case platform_keycode::_0: return '0';
	case platform_keycode::_1: return '1';
	case platform_keycode::_2: return '2';
	case platform_keycode::_3: return '3';
	case platform_keycode::_4: return '4';
	case platform_keycode::_5: return '5';
	case platform_keycode::_6: return '6';
	case platform_keycode::_7: return '7';
	case platform_keycode::_8: return '8';
	case platform_keycode::_9: return '9';
	case platform_keycode::a: return 'A';
	case platform_keycode::b: return 'B';
	case platform_keycode::c: return 'C';
	case platform_keycode::d: return 'D';
	case platform_keycode::e: return 'E';
	case platform_keycode::f: return 'F';
	case platform_keycode::g: return 'G';
	case platform_keycode::h: return 'H';
	case platform_keycode::i: return 'I';
	case platform_keycode::j: return 'J';
	case platform_keycode::k: return 'K';
	case platform_keycode::l: return 'L';
	case platform_keycode::m: return 'M';
	case platform_keycode::n: return 'N';
	case platform_keycode::o: return 'O';
	case platform_keycode::p: return 'P';
	case platform_keycode::q: return 'Q';
	case platform_keycode::r: return 'R';
	case platform_keycode::s: return 'S';
	case platform_keycode::t: return 'T';
	case platform_keycode::u: return 'U';
	case platform_keycode::v: return 'V';
	case platform_keycode::w: return 'W';
	case platform_keycode::x: return 'X';
	case platform_keycode::y: return 'Y';
	case platform_keycode::z: return 'Z';
	case platform_keycode::tab: return VK_TAB;
	case platform_keycode::grave: return VK_OEM_3;
	case platform_keycode::dash: return VK_OEM_MINUS;
	case platform_keycode::comma: return VK_OEM_COMMA;
	case platform_keycode::slash: return VK_OEM_2;
	case platform_keycode::space: return VK_SPACE;
	case platform_keycode::equals: return VK_OEM_PLUS;
	case platform_keycode::enter: return VK_RETURN;
	case platform_keycode::period: return VK_OEM_PERIOD;
	case platform_keycode::rbracket: return VK_OEM_6;
	case platform_keycode::lbracket: return VK_OEM_4;
	case platform_keycode::semicolon: return VK_OEM_1;
	case platform_keycode::backslash: return VK_OEM_5;
	case platform_keycode::np_0: return VK_NUMPAD0;
	case platform_keycode::np_1: return VK_NUMPAD1;
	case platform_keycode::np_2: return VK_NUMPAD2;
	case platform_keycode::np_3: return VK_NUMPAD3;
	case platform_keycode::np_4: return VK_NUMPAD4;
	case platform_keycode::np_5: return VK_NUMPAD5;
	case platform_keycode::np_6: return VK_NUMPAD6;
	case platform_keycode::np_7: return VK_NUMPAD7;
	case platform_keycode::np_8: return VK_NUMPAD8;
	case platform_keycode::np_9: return VK_NUMPAD9;
	case platform_keycode::np_add: return VK_ADD;
	case platform_keycode::np_period: return VK_DECIMAL;
	case platform_keycode::np_divide: return VK_DIVIDE;
	case platform_keycode::np_multiply: return VK_MULTIPLY;
	case platform_keycode::np_subtract: return VK_SUBTRACT;
	case platform_keycode::backspace: return VK_BACK;
	case platform_keycode::capslock: return VK_CAPITAL;
	case platform_keycode::del: return VK_DELETE;
	case platform_keycode::down: return VK_DOWN;
	case platform_keycode::up: return VK_UP;
	case platform_keycode::left: return VK_LEFT;
	case platform_keycode::right: return VK_RIGHT;
	case platform_keycode::end: return VK_END;
	case platform_keycode::escape: return VK_ESCAPE;
	case platform_keycode::f1: return VK_F1;
	case platform_keycode::f2: return VK_F2;
	case platform_keycode::f3: return VK_F3;
	case platform_keycode::f4: return VK_F4;
	case platform_keycode::f5: return VK_F5;
	case platform_keycode::f6: return VK_F6;
	case platform_keycode::f7: return VK_F7;
	case platform_keycode::f8: return VK_F8;
	case platform_keycode::f9: return VK_F9;
	case platform_keycode::f10: return VK_F10;
	case platform_keycode::f11: return VK_F11;
	case platform_keycode::f12: return VK_F12;
	case platform_keycode::home: return VK_HOME;
	case platform_keycode::insert: return VK_INSERT;
	case platform_keycode::lalt: return VK_LMENU;
	case platform_keycode::ralt: return VK_RMENU;
	case platform_keycode::lctrl: return VK_LCONTROL;
	case platform_keycode::rctrl: return VK_RCONTROL;
	case platform_keycode::lshift: return VK_LSHIFT;
	case platform_keycode::rshift: return VK_RSHIFT;
	case platform_keycode::numlock: return VK_NUMLOCK;
	case platform_keycode::pgup: return VK_PRIOR;
	case platform_keycode::pgdown: return VK_NEXT;
	case platform_keycode::scrolllock: return VK_SCROLL;
	default:  return 0;
	}
}

platform_keycode translate_key_code(WPARAM wParam) {
	switch(wParam) {
	case '0': return platform_keycode::_0;
	case '1': return platform_keycode::_1;
	case '2': return platform_keycode::_2;
	case '3': return platform_keycode::_3;
	case '4': return platform_keycode::_4;
	case '5': return platform_keycode::_5;
	case '6': return platform_keycode::_6;
	case '7': return platform_keycode::_7;
	case '8': return platform_keycode::_8;
	case '9': return platform_keycode::_9;
	case 'A': return platform_keycode::a;
	case 'B': return platform_keycode::b;
	case 'C': return platform_keycode::c;
	case 'D': return platform_keycode::d;
	case 'E': return platform_keycode::e;
	case 'F': return platform_keycode::f;
	case 'G': return platform_keycode::g;
	case 'H': return platform_keycode::h;
	case 'I': return platform_keycode::i;
	case 'J': return platform_keycode::j;
	case 'K': return platform_keycode::k;
	case 'L': return platform_keycode::l;
	case 'M': return platform_keycode::m;
	case 'N': return platform_keycode::n;
	case 'O': return platform_keycode::o;
	case 'P': return platform_keycode::p;
	case 'Q': return platform_keycode::q;
	case 'R': return platform_keycode::r;
	case 'S': return platform_keycode::s;
	case 'T': return platform_keycode::t;
	case 'U': return platform_keycode::u;
	case 'V': return platform_keycode::v;
	case 'W': return platform_keycode::w;
	case 'X': return platform_keycode::x;
	case 'Y': return platform_keycode::y;
	case 'Z': return platform_keycode::z;
	case VK_TAB: return platform_keycode::tab;
	case VK_OEM_3: return platform_keycode::grave;
	case VK_OEM_MINUS: return platform_keycode::dash;
	case VK_OEM_COMMA: return platform_keycode::comma;
	case VK_OEM_2: return platform_keycode::slash;
	case VK_SPACE: return platform_keycode::space;
	case VK_OEM_PLUS: return platform_keycode::equals;
	case VK_RETURN: return platform_keycode::enter;
	case VK_OEM_PERIOD: return platform_keycode::period;
	case VK_OEM_6: return platform_keycode::rbracket;
	case VK_OEM_4: return platform_keycode::lbracket;
	case VK_OEM_1: return platform_keycode::semicolon;
	case VK_OEM_5: return platform_keycode::backslash;
	case VK_NUMPAD0: return platform_keycode::np_0;
	case VK_NUMPAD1: return platform_keycode::np_1;
	case VK_NUMPAD2: return platform_keycode::np_2;
	case VK_NUMPAD3: return platform_keycode::np_3;
	case VK_NUMPAD4: return platform_keycode::np_4;
	case VK_NUMPAD5: return platform_keycode::np_5;
	case VK_NUMPAD6: return platform_keycode::np_6;
	case VK_NUMPAD7: return platform_keycode::np_7;
	case VK_NUMPAD8: return platform_keycode::np_8;
	case VK_NUMPAD9: return platform_keycode::np_9;
	case VK_ADD: return platform_keycode::np_add;
	case VK_DECIMAL: return platform_keycode::np_period;
	case VK_DIVIDE: return platform_keycode::np_divide;
	case VK_MULTIPLY: return platform_keycode::np_multiply;
	case VK_SUBTRACT: return platform_keycode::np_subtract;
	case VK_BACK: return platform_keycode::backspace;
	case VK_CAPITAL: return platform_keycode::capslock;
	case VK_DELETE: return platform_keycode::del;
	case VK_DOWN: return platform_keycode::down;
	case VK_UP: return platform_keycode::up;
	case VK_LEFT: return platform_keycode::left;
	case VK_RIGHT: return platform_keycode::right;
	case VK_END: return platform_keycode::end;
	case VK_ESCAPE: return platform_keycode::escape;
	case VK_F1: return platform_keycode::f1;
	case VK_F2: return platform_keycode::f2;
	case VK_F3: return platform_keycode::f3;
	case VK_F4: return platform_keycode::f4;
	case VK_F5: return platform_keycode::f5;
	case VK_F6: return platform_keycode::f6;
	case VK_F7: return platform_keycode::f7;
	case VK_F8: return platform_keycode::f8;
	case VK_F9: return platform_keycode::f9;
	case VK_F10: return platform_keycode::f10;
	case VK_F11: return platform_keycode::f11;
	case VK_F12: return platform_keycode::f12;
	case VK_HOME: return platform_keycode::home;
	case VK_INSERT: return platform_keycode::insert;
	case VK_LMENU: return platform_keycode::lalt;
	case VK_RMENU: return platform_keycode::ralt;
	case VK_LCONTROL: return platform_keycode::lctrl;
	case VK_RCONTROL: return platform_keycode::rctrl;
	case VK_LSHIFT: return platform_keycode::lshift;
	case VK_RSHIFT: return platform_keycode::rshift;
	case VK_NUMLOCK: return platform_keycode::numlock;
	case VK_PRIOR: return platform_keycode::pgup;
	case VK_NEXT: return platform_keycode::pgdown;
	case VK_SCROLL: return platform_keycode::scrolllock;
	default:  return platform_keycode::none;
	}
}

i32 win32_get_scancode(platform_keycode code) {

	return MapVirtualKeyExA(translate_key_code(code), MAPVK_VK_TO_VSC_EX, null);
}

bool win32_keydown(platform_keycode key) {

	u16 vk_code = translate_key_code(key);

	return (GetAsyncKeyState(vk_code) & 0x8000) != 0;
}

LRESULT WINCALLBACK window_proc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	platform_event evt;

	switch(msg) {

		// window messages
		case WM_ACTIVATEAPP: {
			evt.type = platform_event_type::window;
			if(wParam == TRUE) {
				evt.window.op = platform_windowop::focused;
			} else {
				evt.window.op = platform_windowop::unfocused;
			}
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_QUIT:
		case WM_CLOSE:
		/*case WM_DESTROY:*/ {
			evt.type = platform_event_type::window;
			evt.window.op = platform_windowop::close;
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_MOVE: {
			evt.type = platform_event_type::window;
			evt.window.op = platform_windowop::moved;
			evt.window.x = (i16)LOWORD(lParam);
			evt.window.y = (i16)HIWORD(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_SHOWWINDOW: {
			evt.type = platform_event_type::window;
			if(wParam == TRUE) {
				evt.window.op = platform_windowop::shown;
			} else {
				evt.window.op = platform_windowop::hidden;
			}
			switch(lParam) {
			case SW_OTHERUNZOOM:
				evt.window.op = platform_windowop::shown;
				break;
			case SW_OTHERZOOM:
				evt.window.op = platform_windowop::hidden;
				break;
			case SW_PARENTCLOSING:
				evt.window.op = platform_windowop::minimized;
				break;
			case SW_PARENTOPENING:
				evt.window.op = platform_windowop::restored;
				break;
			}
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_SIZE: {
			evt.type = platform_event_type::window;
			switch(wParam) {
			case SIZE_MAXHIDE: return 0;
			case SIZE_MAXIMIZED:
				evt.window.op = platform_windowop::maximized;
				break;
			case SIZE_MAXSHOW: return 0;
			case SIZE_MINIMIZED: 
				evt.window.op = platform_windowop::minimized;
				break;
			case SIZE_RESTORED:
				evt.window.op = platform_windowop::resized;
				break;
			}
			evt.window.x = (i16)LOWORD(lParam);
			evt.window.y = (i16)HIWORD(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		
		// character messages
		case WM_SYSCHAR:
		case WM_CHAR: {
			evt.type = platform_event_type::rune;

			wchar_t rune = (wchar_t)wParam;

			if(WideCharToMultiByte(CP_UTF8, 0, &rune, 1, evt.rune.rune_utf8, 5, null, null))
				global_enqueue(global_enqueue_param, evt);

			return 0;
		} 

		// keyboard messages
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			evt.type = platform_event_type::key;
			evt.key.flags |= (u16)platform_keyflag::press;
			if(wParam == VK_SHIFT) {
				UINT scancode = (lParam & 0x00ff0000) >> 16;
				wParam = MapVirtualKeyA(scancode, MAPVK_VSC_TO_VK_EX);
			} else if(wParam == VK_CONTROL) {
				wParam = (lParam & 0x01000000) != 0 ? VK_RCONTROL : VK_LCONTROL;
			} else if(wParam == VK_MENU) {
				wParam = (lParam & 0x01000000) != 0 ? VK_RMENU : VK_LMENU;
			}
			evt.key.code = translate_key_code(wParam);
			if(lParam & 1<<30) {
				evt.key.flags &= ~(u16)platform_keyflag::press;
				evt.key.flags |= (u16)platform_keyflag::repeat;
			}
			if(GetKeyState(VK_LSHIFT) & 0x8000) 	evt.key.flags |= (u16)platform_keyflag::lshift;
			if(GetKeyState(VK_RSHIFT) & 0x8000) 	evt.key.flags |= (u16)platform_keyflag::rshift;
			if(GetKeyState(VK_LCONTROL) & 0x8000) 	evt.key.flags |= (u16)platform_keyflag::lctrl;
			if(GetKeyState(VK_RCONTROL) & 0x8000) 	evt.key.flags |= (u16)platform_keyflag::rctrl;
			if(GetKeyState(VK_LMENU) & 0x8000) 		evt.key.flags |= (u16)platform_keyflag::lalt;
			if(GetKeyState(VK_RMENU) & 0x8000) 		evt.key.flags |= (u16)platform_keyflag::ralt;
			if(GetKeyState(VK_NUMLOCK) & 1)	evt.key.flags |= (u16)platform_keyflag::numlock_on;
			if(GetKeyState(VK_CAPITAL) & 1)	evt.key.flags |= (u16)platform_keyflag::capslock_on;
			global_enqueue(global_enqueue_param, evt);

			i16 repeat = lParam & 0xFFFF;
			for(i16 i = 1; i < repeat; i++) {
				platform_event r_evt = evt;
				r_evt.key.flags |= (u16)platform_keyflag::repeat;
				evt.key.flags &= ~(u16)platform_keyflag::press;
				if(GetKeyState(VK_LSHIFT) & 0x8000) 	r_evt.key.flags |= (u16)platform_keyflag::lshift;
				if(GetKeyState(VK_RSHIFT) & 0x8000) 	r_evt.key.flags |= (u16)platform_keyflag::rshift;
				if(GetKeyState(VK_LCONTROL) & 0x8000) 	r_evt.key.flags |= (u16)platform_keyflag::lctrl;
				if(GetKeyState(VK_RCONTROL) & 0x8000) 	r_evt.key.flags |= (u16)platform_keyflag::rctrl;
				if(GetKeyState(VK_LMENU) & 0x8000) 		r_evt.key.flags |= (u16)platform_keyflag::lalt;
				if(GetKeyState(VK_RMENU) & 0x8000) 		r_evt.key.flags |= (u16)platform_keyflag::ralt;
				if(GetKeyState(VK_NUMLOCK) & 1)	evt.key.flags |= (u16)platform_keyflag::numlock_on;
				if(GetKeyState(VK_CAPITAL) & 1)	evt.key.flags |= (u16)platform_keyflag::capslock_on;
				global_enqueue(global_enqueue_param, r_evt);
			}
			return 0;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			evt.type = platform_event_type::key;
			evt.key.flags |= (u16)platform_keyflag::release;
			if(wParam == VK_SHIFT) {
				UINT scancode = (lParam & 0x00ff0000) >> 16;
				wParam = MapVirtualKeyA(scancode, MAPVK_VSC_TO_VK_EX);
			} else if(wParam == VK_CONTROL) {
				wParam = (lParam & 0x01000000) != 0 ? VK_RCONTROL : VK_LCONTROL;
			} else if(wParam == VK_MENU) {
				wParam = (lParam & 0x01000000) != 0 ? VK_RMENU : VK_LMENU;
			}
			evt.key.code = translate_key_code(wParam);
			if(GetKeyState(VK_LSHIFT) & 0x8000) 	evt.key.flags |= (u16)platform_keyflag::lshift;
			if(GetKeyState(VK_RSHIFT) & 0x8000) 	evt.key.flags |= (u16)platform_keyflag::rshift;
			if(GetKeyState(VK_LCONTROL) & 0x8000) 	evt.key.flags |= (u16)platform_keyflag::lctrl;
			if(GetKeyState(VK_RCONTROL) & 0x8000) 	evt.key.flags |= (u16)platform_keyflag::rctrl;
			if(GetKeyState(VK_LMENU) & 0x8000) 		evt.key.flags |= (u16)platform_keyflag::lalt;
			if(GetKeyState(VK_RMENU) & 0x8000) 		evt.key.flags |= (u16)platform_keyflag::ralt;
			if(GetKeyState(VK_NUMLOCK) & 1)	evt.key.flags |= (u16)platform_keyflag::numlock_on;
			if(GetKeyState(VK_CAPITAL) & 1)	evt.key.flags |= (u16)platform_keyflag::capslock_on;			
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}

		// mouse messages
		case WM_LBUTTONDOWN: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::press;
			evt.mouse.flags |= (u16)platform_mouseflag::lclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_LBUTTONUP: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::release;
			evt.mouse.flags |= (u16)platform_mouseflag::lclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_LBUTTONDBLCLK: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::dbl;
			evt.mouse.flags |= (u16)platform_mouseflag::lclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;			
		}
		case WM_RBUTTONDOWN: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::press;
			evt.mouse.flags |= (u16)platform_mouseflag::rclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_RBUTTONUP: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::release;
			evt.mouse.flags |= (u16)platform_mouseflag::rclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_RBUTTONDBLCLK: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::dbl;
			evt.mouse.flags |= (u16)platform_mouseflag::rclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;			
		}
		case WM_MBUTTONDOWN: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::press;
			evt.mouse.flags |= (u16)platform_mouseflag::mclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_MBUTTONUP: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::release;
			evt.mouse.flags |= (u16)platform_mouseflag::mclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_MBUTTONDBLCLK: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::dbl;
			evt.mouse.flags |= (u16)platform_mouseflag::mclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_MOUSEWHEEL: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::wheel;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			evt.mouse.w = (u8)(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_XBUTTONDOWN: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::press;
			i16 xbutton = GET_XBUTTON_WPARAM(wParam);
			if(xbutton == XBUTTON1) {
				evt.mouse.flags |= (u16)platform_mouseflag::x1click;
			} else {
				evt.mouse.flags |= (u16)platform_mouseflag::x2click;
			}
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_XBUTTONUP: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::release;
			i16 xbutton = GET_XBUTTON_WPARAM(wParam);
			if(xbutton == XBUTTON1) {
				evt.mouse.flags |= (u16)platform_mouseflag::x1click;
			} else {
				evt.mouse.flags |= (u16)platform_mouseflag::x2click;
			}
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_XBUTTONDBLCLK: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::dbl;
			i16 xbutton = GET_XBUTTON_WPARAM(wParam);
			if(xbutton == XBUTTON1) {
				evt.mouse.flags |= (u16)platform_mouseflag::x1click;
			} else {
				evt.mouse.flags |= (u16)platform_mouseflag::x2click;
			}
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_MOUSEMOVE: {
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::move;

			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);

			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_INPUT: {

			RAWINPUT raw = {};
			u32 size = sizeof(RAWINPUT);
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));
			
			evt.type = platform_event_type::mouse;
			evt.mouse.flags |= (u16)platform_mouseflag::move;

			evt.mouse.x = (i16)raw.mouse.lLastX;
			evt.mouse.y = (i16)raw.mouse.lLastY;

			global_enqueue(global_enqueue_param, evt);
			return 0;
		}

		case WM_SETCURSOR: {
			return LOWORD(lParam) == HTCLIENT ? false : DefWindowProcA(handle, msg, wParam, lParam);
		} break;

		// case WM_NCCREATE: return true;
		// all others
		default: {
			return DefWindowProcA(handle, msg, wParam, lParam);
		}
	}
}

LRESULT WINCALLBACK temp_window_proc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {

	if(msg == WM_CLOSE) {
		return 0;
	} else {
		return DefWindowProcA(handle, msg, wParam, lParam);
	}
}

platform_error win32_wgl_setup() {

	platform_error ret;

	if(wglChoosePixelFormatARB && wglCreateContextAttribsARB && wglSwapIntervalEXT) return ret;

	HINSTANCE instance = GetModuleHandleA(null);

	WNDCLASSEXA temp_class = {
		sizeof(WNDCLASSEXA),
		CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS,
		temp_window_proc,
		0, 0,
		instance,
		0, LoadCursorA(0, IDC_ARROW), 0, 0,
		"temp", 0
	};

	if(RegisterClassExA(&temp_class) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	HANDLE temp_window = CreateWindowExA(WS_EX_ACCEPTFILES, "temp", "temp", WS_OVERLAPPEDWINDOW,
							 			 CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0, instance, 0);

	if(temp_window == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	HDC temp_device_context = GetDC(temp_window);

	if(temp_device_context == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	PIXELFORMATDESCRIPTOR pixel_format = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1, 
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 
		24, 8, 0, 8, 0, 8, 0, 8, 0,
		0, 0, 0, 0, 0,
		24, 8, 0, 0, 0, 0, 0, 0
	};

	int pixel_index = ChoosePixelFormat(temp_device_context, &pixel_format);

	if(pixel_index == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	if(SetPixelFormat(temp_device_context, pixel_index, 0) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	HGLRC gl_temp = wglCreateContext(temp_device_context);

	if(gl_temp == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	if(wglMakeCurrent(temp_device_context, gl_temp) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t)wglGetProcAddress("wglCreateContextAttribsARB");

	if(wglCreateContextAttribsARB == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	wglChoosePixelFormatARB = (wglChoosePixelFormatARB_t)wglGetProcAddress("wglChoosePixelFormatARB");

	if(wglChoosePixelFormatARB == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	wglSwapIntervalEXT = (wglSwapIntervalEXT_t)wglGetProcAddress("wglSwapIntervalEXT");

	if(wglSwapIntervalEXT == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	if(wglMakeCurrent(temp_device_context, null) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	if(wglDeleteContext(gl_temp) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	if(DestroyWindow(temp_window) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	return ret;
}

platform_error win32_recreate_window(platform_window* window) {

	platform_error ret;

	ret = win32_destroy_window(window);
	if(!ret.good) return ret;

	ret = win32_create_window(window);

	if(ret.good) {
		platform_event evt;
		evt.type = platform_event_type::window;
		evt.window.op = platform_windowop::recreate;
		global_enqueue(global_enqueue_param, evt);
	}

	return ret;
}

platform_error win32_create_window(platform_window* window) {

	platform_error ret;

	string title = make_cat_string(string_from_c_str(window->settings.c_title), str(" | Win32"), &win32_heap_alloc);

	HINSTANCE 	instance = GetModuleHandleA(null);
	HANDLE 		process  = GetCurrentProcess();

	SetPriorityClass(process, ABOVE_NORMAL_PRIORITY_CLASS);

	if(instance == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	platform_error setup = win32_wgl_setup();
	if(!setup.good) {
		return setup;
	}

	window->window_class = {
		sizeof(WNDCLASSEXA),
		CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS,
		window_proc,
		0, 0,
		instance,
		0, LoadCursorA(0, IDC_ARROW), 0, 0,
		window->settings.c_title, 0
	};

	if(RegisterClassExA(&window->window_class) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	window->handle = CreateWindowExA(WS_EX_ACCEPTFILES, window->settings.c_title, title.c_str, WS_OVERLAPPEDWINDOW,
									 CW_USEDEFAULT, CW_USEDEFAULT, window->settings.w, window->settings.h, 0, 0, instance, 0);

	if(window->handle == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	window->device_context = GetDC(window->handle);

	if(window->device_context == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	f32 pfdf_attribs[] = {0.0f};

	i32 pfdi_attribs[] = {
		(i32)wgl_context::draw_to_window_arb, 1,
		(i32)wgl_context::support_opengl_arb, 1,
		(i32)wgl_context::double_buffer_arb,  1,
		(i32)wgl_context::pixel_type_arb,	  (i32)wgl_context::type_rgba_arb,
		(i32)wgl_context::color_bits_arb,	  24,
		(i32)wgl_context::alpha_bits_arb, 	  8,
		(i32)wgl_context::depth_bits_arb,	  24,
		(i32)wgl_context::stencil_bits_arb,	  8,
		(i32)wgl_context::sample_buffers_arb, window->settings.samples > 1 ? 1 : 0,
		(i32)wgl_context::samples_arb,		  window->settings.samples > 1 ? window->settings.samples : 0,
		0,
	};

	u32 match = 0;
	i32 pixel_index = 0;
	BOOL result = wglChoosePixelFormatARB(window->device_context, pfdi_attribs, pfdf_attribs, 1, &pixel_index, &match);
	if(!result || !match)	 {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	if(SetPixelFormat(window->device_context, pixel_index, null) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	i32 context_attribs[] = {
		(i32)wgl_context::major_version_arb, 	4,
		(i32)wgl_context::minor_version_arb, 	6,
		(i32)wgl_context::flags_arb, 			(i32)wgl_context::forward_compatible_bit_arb,
		(i32)wgl_context::profile_mask_arb, 	(i32)wgl_context::core_profile_bit_arb,
		0
	};

#define TRY_VERSION(MAJOR, MINOR) context_attribs[1] = MAJOR; context_attribs[3] = MINOR; \
								  if(window->gl_context == null) window->gl_context = wglCreateContextAttribsARB(window->device_context, 0, context_attribs);

	TRY_VERSION(4, 6);
	TRY_VERSION(4, 5);
	TRY_VERSION(4, 4);
	TRY_VERSION(4, 3);
	TRY_VERSION(4, 2);
	TRY_VERSION(4, 1);
	TRY_VERSION(4, 0);
	TRY_VERSION(3, 3);
	TRY_VERSION(3, 2);
	TRY_VERSION(3, 1);
	TRY_VERSION(3, 0);
	TRY_VERSION(2, 1);
	TRY_VERSION(2, 0);

#undef TRY_VERSION

	if(window->gl_context == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	if(wglMakeCurrent(window->device_context, window->gl_context) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	wglSwapIntervalEXT(window->settings.vsync ? 1 : 0);

	ShowWindowAsync(window->handle, SW_SHOW);

	free_string(title, win32_heap_free);

	return ret;
}

platform_error win32_destroy_window(platform_window* window) {

	platform_error ret;

	if(wglDeleteContext(window->gl_context) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}
	window->gl_context = null;
	if(ReleaseDC(window->handle, window->device_context) == 0) {
		ret.good = false;
		ret.error = GetLastError();
	}
	window->device_context = null;
	if(DestroyWindow(window->handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}
	window->handle = null;
	if(UnregisterClassA(window->settings.c_title, null) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	return ret;
}
