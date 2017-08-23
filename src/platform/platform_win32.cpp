
#include "platform_win32.h"

extern "C" {
    // Request dGPU
    __declspec(dllexport) bool NvOptimusEnablement = true;
    __declspec(dllexport) bool AmdPowerXpressRequestHighPerformance = true;
}

typedef HGLRC (WINAPI *wglCreateContextAttribsARB_t)(HDC hDC, HGLRC hShareContext,
													const int* attribList);
typedef BOOL  (WINAPI *wglSwapIntervalEXT_t)(int interval);

static wglCreateContextAttribsARB_t wglCreateContextAttribsARB;
static wglSwapIntervalEXT_t 		wglSwapIntervalEXT;

static void (*global_enqueue)(void* queue_param, platform_event evt) = null;
static void* global_enqueue_param = null;

platform_api platform_build_api() {

	platform_api ret;

	ret.platform_create_window			= &win32_create_window;
	ret.platform_destroy_window			= &win32_destroy_window;
	ret.platform_swap_buffers			= &win32_swap_buffers;
	ret.platform_set_queue_callback		= &win32_set_queue_callback;
	ret.platform_pump_events			= &win32_pump_events;
	ret.platform_queue_event 			= &win32_queue_event;
	ret.platform_wait_message			= &win32_wait_message;
	ret.platform_load_library			= &win32_load_library;
	ret.platform_free_library			= &win32_free_library;
	ret.platform_get_proc_address 		= &win32_get_proc_address;
	ret.platform_get_file_attributes 	= &win32_get_file_attributes;
	ret.platform_test_file_written		= &win32_test_file_written;
	ret.platform_copy_file				= &win32_copy_file;
	ret.platform_heap_alloc				= &win32_heap_alloc;
	ret.platform_heap_realloc			= &win32_heap_realloc;
	ret.platform_heap_free				= &win32_heap_free;
	ret.platform_get_bin_path			= &win32_get_bin_path;
	ret.platform_create_thread			= &win32_create_thread;
	ret.platform_this_thread_id			= &win32_this_thread_id;
	ret.platform_terminate_thread		= &win32_terminate_thread;
	ret.platform_exit_this_thread		= &win32_exit_this_thread;
	ret.platform_thread_sleep			= &win32_thread_sleep;
	ret.platform_create_semaphore		= &win32_create_semaphore;
	ret.platform_destroy_semaphore		= &win32_destroy_semaphore;
	ret.platform_signal_semaphore		= &win32_signal_semaphore;
	ret.platform_wait_semaphore			= &win32_wait_semaphore;
	ret.platform_create_mutex			= &win32_create_mutex;
	ret.platform_destroy_mutex			= &win32_destroy_mutex;
	ret.platform_aquire_mutex			= &win32_aquire_mutex;
	ret.platform_try_aquire_mutex		= &win32_try_aquire_mutex;
	ret.platform_release_mutex			= &win32_release_mutex;
	ret.platform_destroy_thread			= &win32_destroy_thread;
	ret.platform_get_num_cpus			= &win32_get_num_cpus;
	ret.platform_join_thread			= &win32_join_thread;
	ret.platform_create_file			= &win32_create_file;
	ret.platform_close_file				= &win32_close_file;
	ret.platform_write_file				= &win32_write_file;
	ret.platform_read_file				= &win32_read_file;
	ret.platform_get_stdout_as_file		= &win32_get_stdout_as_file;
	ret.platform_get_timef				= &win32_get_timef;
	ret.platform_make_timef				= &win32_make_timef;
	ret.platform_get_window_size		= &win32_get_window_size;
	ret.platform_write_stdout			= &win32_write_stdout;
	ret.platform_file_size				= &win32_file_size;
	ret.platform_get_glproc				= &win32_get_glproc;
	ret.platform_keydown				= &win32_keydown;
	ret.platform_is_debugging			= &win32_is_debugging;

	return ret;
}

bool win32_is_debugging() {
	return IsDebuggerPresent() == TRUE;
}

platform_error win32_write_stdout(string str) {

	platform_error ret;

	if(WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), (void*)str.c_str, str.len, 0, 0) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

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

void win32_get_timef(string fmt, string* out) {

	i32 len = GetTimeFormatA(LOCALE_USER_DEFAULT, 0, null, fmt.c_str, null, 0);

	out->len = len;

	GetTimeFormatA(LOCALE_USER_DEFAULT, 0, null, fmt.c_str, out->c_str, len);
}

string win32_make_timef(string fmt) {

	i32 len = GetTimeFormatA(LOCALE_USER_DEFAULT, 0, null, fmt.c_str, null, 0);

	string ret = make_string(len, &win32_heap_alloc);
	ret.len = ret.cap;

	GetTimeFormatA(LOCALE_USER_DEFAULT, 0, null, fmt.c_str, ret.c_str, len);	

	return ret;
}

platform_error win32_get_stdout_as_file(platform_file* file) {

	platform_error ret;

	HANDLE std = GetStdHandle(STD_OUTPUT_HANDLE);

	if(std == INVALID_HANDLE_VALUE || std == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	file->handle = std;

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
		case platform_file_open_op::existing_or_create:
		creation = OPEN_ALWAYS;
		break;
		case platform_file_open_op::create:
		creation = CREATE_ALWAYS;
		break;
		case platform_file_open_op::clear_existing:
		creation = TRUNCATE_EXISTING;
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
}

void win32_aquire_mutex(platform_mutex* mut) {

	EnterCriticalSection(&mut->cs);
}

void win32_try_aquire_mutex(platform_mutex* mut) {

	TryEnterCriticalSection(&mut->cs);
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

platform_error win32_terminate_thread(platform_thread* thread, i32 exit_code) {

	platform_error ret;

	if(TerminateThread(thread->handle, (DWORD)exit_code) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;
}

void win32_exit_this_thread(i32 exit_code) {
	
	ExitThread((DWORD)exit_code);
}

platform_thread_id win32_this_thread_id() {

	platform_thread_id ret;

	ret.id = GetCurrentThreadId();

	return ret;
}

platform_error win32_create_thread(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended) {

	platform_error ret;

	thread->handle = CreateThread(null, 0, (LPTHREAD_START_ROUTINE)proc, param, start_suspended ? CREATE_SUSPENDED : 0, &thread->id.id);

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

// #include <iostream>
// using std::cout;
// using std::endl;

void* win32_heap_alloc(u64 bytes) {

	HANDLE heap = GetProcessHeap();
	void* ret = HeapAlloc(heap, HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, (SIZE_T)bytes);

	// cout << "AAAAAAAAAA " << ret << endl;

	return ret;
}

void* win32_heap_realloc(void* mem, u64 bytes) {

	HANDLE heap = GetProcessHeap();
	void* ret = HeapReAlloc(heap, HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, mem, (SIZE_T)bytes);

	// cout << "RRRRRRRRRR " << ret << endl;

	return ret;
}

void win32_heap_free(void* mem) {

	// cout << "FFFFFFFFFF " << mem << endl;
	// *(u8*)mem = 0;

	HANDLE heap = GetProcessHeap();
	HeapFree(heap, 0, mem);
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

bool win32_keydown(platform_keycode key) {

	u16 vk_code = translate_key_code(key);

	return (GetKeyState(vk_code) & 0x8000) != 0;
}

LRESULT CALLBACK window_proc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
	
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
		case WM_DESTROY: {
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

		// keyboard messages
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			evt.type = platform_event_type::key;
			evt.key.flags |= (u16)platform_keyflag::press;
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

		// all others
		default: {
			return DefWindowProcA(handle, msg, wParam, lParam);
		}
	}
}

platform_error win32_create_window(platform_window* window, string title, u32 width, u32 height) {

	platform_error ret;

	window->title = title;
	window->w = width;
	window->h = height;

	HINSTANCE 	instance = GetModuleHandleA(null);
	HGLRC 		gl_temp  = {};
	HANDLE 		process  = GetCurrentProcess();

	SetPriorityClass(process, ABOVE_NORMAL_PRIORITY_CLASS);

	if(instance == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	window->window_class = {
		sizeof(WNDCLASSEXA),
		CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS,
		window_proc,
		0, 0,
		instance,
		0, LoadCursorA(0, IDC_ARROW), 0, 0,
		window->title.c_str, 0
	};

	if(RegisterClassExA(&window->window_class) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	window->handle = CreateWindowExA(WS_EX_ACCEPTFILES, window->title.c_str, window->title.c_str, WS_OVERLAPPEDWINDOW,
			           				 CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, instance, 0);

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

	gl_temp = wglCreateContext(window->device_context);

	if(gl_temp == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	if(wglMakeCurrent(window->device_context, gl_temp) == 0) {
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

	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t)wglGetProcAddress("wglCreateContextAttribsARB");

	if(wglCreateContextAttribsARB == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	if(wglDeleteContext(gl_temp) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	window->gl_context = wglCreateContextAttribsARB(window->device_context, 0, &attribs[0]);

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

	wglSwapIntervalEXT = (wglSwapIntervalEXT_t)wglGetProcAddress("wglSwapIntervalEXT");

	if(wglSwapIntervalEXT == null) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	// TODO(max): vsync/fullscreen/AA/etc settings
		// https://blogs.msdn.microsoft.com/oldnewthing/20100412-00/?p=14353
	wglSwapIntervalEXT(0);

	ShowWindowAsync(window->handle, SW_SHOW);

	return ret;
}

platform_error win32_destroy_window(platform_window* window) {

	platform_error ret;

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