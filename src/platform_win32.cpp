
void (*global_enqueue)(void* queue_param, platform_event evt) = NULL;
void* global_enqueue_param = NULL;

platform_api platform_build_api() {

	platform_api ret;

	ret.platform_create_window			= &platform_create_window;
	ret.platform_destroy_window			= &platform_destroy_window;
	ret.platform_swap_buffers			= &platform_swap_buffers;
	ret.platform_set_queue_callback		= &platform_set_queue_callback;
	ret.platform_queue_messages			= &platform_queue_messages;
	ret.platform_wait_message			= &platform_wait_message;
	ret.platform_load_library			= &platform_load_library;
	ret.platform_free_library			= &platform_free_library;
	ret.platform_get_proc_address 		= &platform_get_proc_address;
	ret.platform_get_file_attributes 	= &platform_get_file_attributes;
	ret.platform_test_file_written		= &platform_test_file_written;
	ret.platform_copy_file				= &platform_copy_file;
	ret.platform_heap_alloc				= &platform_heap_alloc;
	ret.platform_heap_free				= &platform_heap_free;
	ret.platform_get_bin_path			= &platform_get_bin_path;
	ret.platform_create_thread			= &platform_create_thread;
	ret.platform_this_thread_id			= &platform_this_thread_id;
	ret.platform_terminate_thread		= &platform_terminate_thread;
	ret.platform_exit_this_thread		= &platform_exit_this_thread;
	ret.platform_thread_sleep			= &platform_thread_sleep;
	ret.platform_create_semaphore		= &platform_create_semaphore;
	ret.platform_destroy_semaphore		= &platform_destroy_semaphore;
	ret.platform_signal_semaphore		= &platform_signal_semaphore;
	ret.platform_wait_semaphore			= &platform_wait_semaphore;
	ret.platform_create_mutex			= &platform_create_mutex;
	ret.platform_destroy_mutex			= &platform_destroy_mutex;
	ret.platform_aquire_mutex			= &platform_aquire_mutex;
	ret.platform_release_mutex			= &platform_release_mutex;
	ret.platform_destroy_thread			= &platform_destroy_thread;
	ret.platform_get_num_cpus			= &platform_get_num_cpus;
	ret.platform_join_thread			= &platform_join_thread;
	ret.platform_create_file			= &platform_create_file;
	ret.platform_close_file				= &platform_close_file;
	ret.platform_write_file				= &platform_write_file;
	ret.platform_read_file				= &platform_read_file;
	ret.platform_get_stdout_as_file		= &platform_get_stdout_as_file;
	ret.platform_get_timef				= &platform_get_timef;
	ret.platform_make_timef				= &platform_make_timef;
	ret.platform_get_window_size		= &platform_get_window_size;
	ret.platform_write_stdout			= &platform_write_stdout;
	ret.platform_file_size				= &platform_file_size;

	return ret;
}

platform_error platform_write_stdout(string str) {

	platform_error ret;

	if(WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (void*)str.c_str, str.len, 0, 0) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;	
	}

	return ret;
}

platform_error platform_get_window_size(platform_window* window, i32* w, i32* h) {

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

void platform_get_timef(string fmt, string* out) {

	i32 len = GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, fmt.c_str, NULL, 0);

	out->len = len;

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, fmt.c_str, out->c_str, len);
}

string platform_make_timef(string fmt) {

	i32 len = GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, fmt.c_str, NULL, 0);

	string ret = make_string(len, &platform_heap_alloc);
	ret.len = ret.cap;

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, fmt.c_str, ret.c_str, len);	

	return ret;
}

platform_error platform_get_stdout_as_file(platform_file* file) {

	platform_error ret;

	HANDLE std = GetStdHandle(STD_OUTPUT_HANDLE);

	if(std == INVALID_HANDLE_VALUE || std == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	file->handle = std;

	return ret;
}

u32 platform_file_size(platform_file* file) {
	return GetFileSize(file->handle, NULL);
}

platform_error platform_read_file(platform_file* file, void* mem, u32 bytes) {

	platform_error ret;

	if(ReadFile(file->handle, mem, bytes, NULL, NULL) == FALSE) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	return ret;
}

platform_error platform_write_file(platform_file* file, void* mem, u32 bytes) {

	platform_error ret;

	if(WriteFile(file->handle, mem, (DWORD)bytes, NULL, NULL) == FALSE) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;
}

platform_error platform_create_file(platform_file* file, string path, platform_file_open_op mode) {

	platform_error ret;

	DWORD creation = 0;
	switch(mode) {
		case open_file_existing:
		creation = OPEN_EXISTING;
		break;
		case open_file_existing_or_create:
		creation = OPEN_ALWAYS;
		break;
		case open_file_create:
		creation = CREATE_ALWAYS;
		break;
		case open_file_clear_existing:
		creation = TRUNCATE_EXISTING;
		break;
		default:
		ret.good = false;
		return ret;	
		break;
	}

	file->handle = CreateFileA(path.c_str, GENERIC_READ | GENERIC_WRITE, 0, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file->handle == INVALID_HANDLE_VALUE) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	file->path = path;

	return ret;
}

platform_error platform_close_file(platform_file* file) {

	platform_error ret;

	if(CloseHandle(file->handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	file->handle = NULL;

	return ret;
}

platform_thread_join_state platform_join_thread(platform_thread* thread, i32 ms) {
	
	platform_thread_join_state ret;

	switch(WaitForSingleObject(thread->handle, ms == -1 ? INFINITE : (DWORD)ms)) {
	case WAIT_OBJECT_0:
		ret.state = thread_joined;
		return ret;
	case WAIT_TIMEOUT:
		ret.state = thread_timed_out;
		return ret;
	case WAIT_FAILED:
		ret.state = thread_failed;
		ret.error.good = false;
		ret.error.error = GetLastError();
		return ret;		
	}

	return ret;
}

i32 platform_get_num_cpus() {

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
}

platform_error platform_create_semaphore(platform_semaphore* sem, i32 initial_count, i32 max_count) {

	platform_error ret;

	sem->handle = CreateSemaphoreExA(NULL, initial_count, max_count, NULL, 0, SEMAPHORE_ALL_ACCESS);

	if(sem->handle == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

platform_error platform_destroy_semaphore(platform_semaphore* sem) {
	
	platform_error ret;

	if(CloseHandle(sem->handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	sem->handle = NULL;

	return ret;
}

platform_error platform_signal_semaphore(platform_semaphore* sem, i32 times) {

	platform_error ret;

	if(ReleaseSemaphore(sem->handle, (LONG)times, NULL) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;
}

platform_semaphore_state platform_wait_semaphore(platform_semaphore* sem, i32 ms) {

	platform_semaphore_state ret;

	switch(WaitForSingleObject(sem->handle, ms == -1 ? INFINITE : (DWORD)ms)) {
	case WAIT_OBJECT_0:
		ret.state = semaphore_signaled;
		return ret;
	case WAIT_TIMEOUT:
		ret.state = semaphore_timed_out;
		return ret;
	case WAIT_FAILED:
		ret.state = semaphore_failed;
		ret.error.good = false;
		ret.error.error = GetLastError();
		return ret;		
	}

	return ret;
}

platform_error platform_create_mutex(platform_mutex* mut, bool aquire) {

	platform_error ret;

	mut->handle = CreateMutexExA(NULL, NULL, aquire ? CREATE_MUTEX_INITIAL_OWNER : 0, MUTEX_ALL_ACCESS);

	if(mut->handle == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

platform_error platform_destroy_mutex(platform_mutex* mut) {
	
	platform_error ret;

	if(CloseHandle(mut->handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	mut->handle = NULL;

	return ret;
}

platform_mutex_state platform_aquire_mutex(platform_mutex* mut, i32 ms) {

	platform_mutex_state ret;

	switch(WaitForSingleObject(mut->handle, ms == -1 ? INFINITE : (DWORD)ms)) {
	case WAIT_ABANDONED:
		ret.state = mutex_abandoned;
		return ret;
	case WAIT_OBJECT_0:
		ret.state = mutex_aquired;
		return ret;
	case WAIT_TIMEOUT:
		ret.state = mutex_timed_out;
		return ret;
	case WAIT_FAILED:
		ret.state = mutex_failed;
		ret.error.good = false;
		ret.error.error = GetLastError();
		return ret;		
	}

	return ret;
}

platform_error platform_release_mutex(platform_mutex* mut) {

	platform_error ret;

	if(ReleaseMutex(mut->handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;	
}

platform_error platform_destroy_thread(platform_thread* thread) {

	platform_error ret;

	if(CloseHandle(thread->handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	thread->handle = NULL;

	return ret;
}

void platform_thread_sleep(i32 ms) {
	Sleep(ms);
}

platform_error platform_terminate_thread(platform_thread* thread, i32 exit_code) {

	platform_error ret;

	if(TerminateThread(thread->handle, (DWORD)exit_code) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;		
	}

	return ret;
}

void platform_exit_this_thread(i32 exit_code) {
	
	ExitThread((DWORD)exit_code);
}

platform_thread_id platform_this_thread_id() {

	platform_thread_id ret;

	ret.id = GetCurrentThreadId();

	return ret;
}

platform_error platform_create_thread(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended) {

	platform_error ret;

	thread->handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)proc, param, start_suspended ? CREATE_SUSPENDED : 0, &thread->id.id);

	if(thread->handle == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;			
	}

	return ret;
}

platform_error platform_get_bin_path(string* path) {

	platform_error ret;

	*path = make_string(MAX_PATH, &platform_heap_alloc);

	DWORD len = GetModuleFileNameA(NULL, (LPSTR)path->c_str, MAX_PATH);

	if(len == 0) {
		ret.good = false;
		ret.error = GetLastError();

		free_string(*path, &platform_heap_free);

		return ret;
	}

	if(path->c_str[len - 1] == '\0') {
		ret.good = false;
		ret.error = GetLastError();

		free_string(*path, &platform_heap_free);

		return ret;
	}

	path->len = len;

	return ret;
}

// #include <iostream>
// using std::cout;
// using std::endl;

void* platform_heap_alloc(u64 bytes) {

	// cout << "alloc" << endl;

	HANDLE heap = GetProcessHeap();
	return HeapAlloc(heap, HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, bytes);
}

void platform_heap_free(void* mem) {

	// cout << "free" << endl;

	// *(u8*)mem = 0;
	HANDLE heap = GetProcessHeap();
	HeapFree(heap, 0, mem);
}

platform_error platform_copy_file(string source, string dest, bool overwrite) {

	platform_error ret;

	if(CopyFile(source.c_str, dest.c_str, !overwrite) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;			
	}

	return ret;
}

bool platform_test_file_written(platform_file_attributes* first, platform_file_attributes* second) {

	return CompareFileTime(&first->attrib.ftLastWriteTime, &second->attrib.ftLastWriteTime) == -1;
}

platform_error platform_get_file_attributes(platform_file_attributes* attrib, string file_path) {

	platform_error ret;

	if(GetFileAttributesExA(file_path.c_str, GetFileExInfoStandard, (LPVOID)&attrib->attrib) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

platform_error platform_get_proc_address(void** address, platform_dll* dll, string name) {

	platform_error ret;

	*address = GetProcAddress(dll->dll_handle, name.c_str);

	if(*address == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

platform_error platform_load_library(platform_dll* dll, string file_path) {

	platform_error ret;

	dll->dll_handle = LoadLibraryA(file_path.c_str);

	if(dll->dll_handle == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

platform_error platform_free_library(platform_dll* dll) {
	
	platform_error ret;

	if(FreeLibrary(dll->dll_handle) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;				
	}

	return ret;
}

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

// we may need to enqueue events both here and in window_proc

void platform_set_queue_callback(void (*enqueue)(void* queue_param, platform_event evt), void* queue_param) {

	global_enqueue = enqueue;
	global_enqueue_param = queue_param;
}

void platform_queue_messages(platform_window* window) {
	
	MSG msg;

	while(PeekMessageA(&msg, window->handle, 0, 0, PM_REMOVE) != 0) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

platform_keycode translate_key_code(WPARAM wParam) {
	switch(wParam) {
	case '0': return key_0;
	case '1': return key_1;
	case '2': return key_2;
	case '3': return key_3;
	case '4': return key_4;
	case '5': return key_5;
	case '6': return key_6;
	case '7': return key_7;
	case '8': return key_8;
	case '9': return key_9;
	case 'A': return key_a;
	case 'B': return key_b;
	case 'C': return key_c;
	case 'D': return key_d;
	case 'E': return key_e;
	case 'F': return key_f;
	case 'G': return key_g;
	case 'H': return key_h;
	case 'I': return key_i;
	case 'J': return key_j;
	case 'K': return key_k;
	case 'L': return key_l;
	case 'M': return key_m;
	case 'N': return key_n;
	case 'O': return key_o;
	case 'P': return key_p;
	case 'Q': return key_q;
	case 'R': return key_r;
	case 'S': return key_s;
	case 'T': return key_t;
	case 'U': return key_u;
	case 'V': return key_v;
	case 'W': return key_w;
	case 'X': return key_x;
	case 'Y': return key_y;
	case 'Z': return key_z;
	case VK_TAB: return key_tab;
	case VK_OEM_3: return key_grave;
	case VK_OEM_MINUS: return key_dash;
	case VK_OEM_COMMA: return key_comma;
	case VK_OEM_2: return key_slash;
	case VK_SPACE: return key_space;
	case VK_OEM_PLUS: return key_equals;
	case VK_RETURN: return key_enter;
	case VK_OEM_PERIOD: return key_period;
	case VK_OEM_6: return key_rbracket;
	case VK_OEM_4: return key_lbracket;
	case VK_OEM_1: return key_semicolon;
	case VK_OEM_5: return key_backslash;
	case VK_NUMPAD0: return key_np_0;
	case VK_NUMPAD1: return key_np_1;
	case VK_NUMPAD2: return key_np_2;
	case VK_NUMPAD3: return key_np_3;
	case VK_NUMPAD4: return key_np_4;
	case VK_NUMPAD5: return key_np_5;
	case VK_NUMPAD6: return key_np_6;
	case VK_NUMPAD7: return key_np_7;
	case VK_NUMPAD8: return key_np_8;
	case VK_NUMPAD9: return key_np_9;
	case VK_ADD: return key_np_add;
	case VK_DECIMAL: return key_np_period;
	case VK_DIVIDE: return key_np_divide;
	case VK_MULTIPLY: return key_np_multiply;
	case VK_SUBTRACT: return key_np_subtract;
	case VK_BACK: return key_backspace;
	case VK_CAPITAL: return key_capslock;
	case VK_DELETE: return key_delete;
	case VK_DOWN: return key_down;
	case VK_UP: return key_up;
	case VK_LEFT: return key_left;
	case VK_RIGHT: return key_right;
	case VK_END: return key_end;
	case VK_ESCAPE: return key_escape;
	case VK_F1: return key_f1;
	case VK_F2: return key_f2;
	case VK_F3: return key_f3;
	case VK_F4: return key_f4;
	case VK_F5: return key_f5;
	case VK_F6: return key_f6;
	case VK_F7: return key_f7;
	case VK_F8: return key_f8;
	case VK_F9: return key_f9;
	case VK_F10: return key_f10;
	case VK_F11: return key_f11;
	case VK_F12: return key_f12;
	case VK_HOME: return key_home;
	case VK_INSERT: return key_insert;
	case VK_LMENU: return key_lalt;
	case VK_RMENU: return key_ralt;
	case VK_LCONTROL: return key_lctrl;
	case VK_RCONTROL: return key_rctrl;
	case VK_LSHIFT: return key_lshift;
	case VK_RSHIFT: return key_rshift;
	case VK_NUMLOCK: return key_numlock;
	case VK_PRIOR: return key_pgup;
	case VK_NEXT: return key_pgdown;
	case VK_SCROLL: return key_scrolllock;
	default:  return key_none;
	}
}

LRESULT CALLBACK window_proc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	platform_event evt;

	switch(msg) {

		// window messages
		case WM_ACTIVATEAPP: {
			evt.type = event_window;
			if(wParam == TRUE) {
				evt.window.op = window_focused;
			} else {
				evt.window.op = window_unfocused;
			}
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_QUIT:
		case WM_CLOSE:
		case WM_DESTROY: {
			evt.type = event_window;
			evt.window.op = window_close;
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_MOVE: {
			evt.type = event_window;
			evt.window.op = window_moved;
			evt.window.x = (i16)LOWORD(lParam);
			evt.window.y = (i16)HIWORD(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_SHOWWINDOW: {
			evt.type = event_window;
			if(wParam == TRUE) {
				evt.window.op = window_shown;
			} else {
				evt.window.op = window_hidden;
			}
			switch(lParam) {
			case SW_OTHERUNZOOM:
				evt.window.op = window_shown;
				break;
			case SW_OTHERZOOM:
				evt.window.op = window_hidden;
				break;
			case SW_PARENTCLOSING:
				evt.window.op = window_minimized;
				break;
			case SW_PARENTOPENING:
				evt.window.op = window_restored;
				break;
			}
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_SIZE: {
			evt.type = event_window;
			switch(wParam) {
			case SIZE_MAXHIDE: return 0;
			case SIZE_MAXIMIZED:
				evt.window.op = window_maximized;
				break;
			case SIZE_MAXSHOW: return 0;
			case SIZE_MINIMIZED: 
				evt.window.op = window_minimized;
				break;
			case SIZE_RESTORED:
				evt.window.op = window_resized;
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
			evt.type = event_key;
			evt.key.flags |= key_flag_press;
			evt.key.code = translate_key_code(wParam);
			if(lParam & 1<<30) {
				evt.key.flags &= ~key_flag_press;
				evt.key.flags |= key_flag_repeat;
			}
			if(GetKeyState(VK_LSHIFT) & 0x8000) 	evt.key.flags |= key_flag_lshift;
			if(GetKeyState(VK_RSHIFT) & 0x8000) 	evt.key.flags |= key_flag_rshift;
			if(GetKeyState(VK_LCONTROL) & 0x8000) 	evt.key.flags |= key_flag_lctrl;
			if(GetKeyState(VK_RCONTROL) & 0x8000) 	evt.key.flags |= key_flag_rctrl;
			if(GetKeyState(VK_LMENU) & 0x8000) 		evt.key.flags |= key_flag_lalt;
			if(GetKeyState(VK_RMENU) & 0x8000) 		evt.key.flags |= key_flag_ralt;
			if(GetKeyState(VK_NUMLOCK) & 1)	evt.key.flags |= key_flag_numlock_on;
			if(GetKeyState(VK_CAPITAL) & 1)	evt.key.flags |= key_flag_capslock_on;
			global_enqueue(global_enqueue_param, evt);

			i16 repeat = lParam & 0xFFFF;
			for(i16 i = 1; i < repeat; i++) {
				platform_event r_evt = evt;
				r_evt.key.flags |= key_flag_repeat;
				evt.key.flags &= ~key_flag_press;
				if(GetKeyState(VK_LSHIFT) & 0x8000) 	r_evt.key.flags |= key_flag_lshift;
				if(GetKeyState(VK_RSHIFT) & 0x8000) 	r_evt.key.flags |= key_flag_rshift;
				if(GetKeyState(VK_LCONTROL) & 0x8000) 	r_evt.key.flags |= key_flag_lctrl;
				if(GetKeyState(VK_RCONTROL) & 0x8000) 	r_evt.key.flags |= key_flag_rctrl;
				if(GetKeyState(VK_LMENU) & 0x8000) 		r_evt.key.flags |= key_flag_lalt;
				if(GetKeyState(VK_RMENU) & 0x8000) 		r_evt.key.flags |= key_flag_ralt;
				if(GetKeyState(VK_NUMLOCK) & 1)	evt.key.flags |= key_flag_numlock_on;
				if(GetKeyState(VK_CAPITAL) & 1)	evt.key.flags |= key_flag_capslock_on;
				global_enqueue(global_enqueue_param, r_evt);
			}
			return 0;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			evt.type = event_key;
			evt.key.flags |= key_flag_release;
			evt.key.code = translate_key_code(wParam);
			if(GetKeyState(VK_LSHIFT) & 0x8000) 	evt.key.flags |= key_flag_lshift;
			if(GetKeyState(VK_RSHIFT) & 0x8000) 	evt.key.flags |= key_flag_rshift;
			if(GetKeyState(VK_LCONTROL) & 0x8000) 	evt.key.flags |= key_flag_lctrl;
			if(GetKeyState(VK_RCONTROL) & 0x8000) 	evt.key.flags |= key_flag_rctrl;
			if(GetKeyState(VK_LMENU) & 0x8000) 		evt.key.flags |= key_flag_lalt;
			if(GetKeyState(VK_RMENU) & 0x8000) 		evt.key.flags |= key_flag_ralt;
			if(GetKeyState(VK_NUMLOCK) & 1)	evt.key.flags |= key_flag_numlock_on;
			if(GetKeyState(VK_CAPITAL) & 1)	evt.key.flags |= key_flag_capslock_on;			
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}

		// mouse messages
		case WM_LBUTTONDOWN: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_press;
			evt.mouse.flags |= mouse_flag_lclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_LBUTTONUP: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_release;
			evt.mouse.flags |= mouse_flag_lclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_LBUTTONDBLCLK: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_double;
			evt.mouse.flags |= mouse_flag_lclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;			
		}
		case WM_RBUTTONDOWN: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_press;
			evt.mouse.flags |= mouse_flag_rclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_RBUTTONUP: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_release;
			evt.mouse.flags |= mouse_flag_rclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_RBUTTONDBLCLK: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_double;
			evt.mouse.flags |= mouse_flag_rclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;			
		}
		case WM_MBUTTONDOWN: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_press;
			evt.mouse.flags |= mouse_flag_mclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_MBUTTONUP: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_release;
			evt.mouse.flags |= mouse_flag_mclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_MBUTTONDBLCLK: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_double;
			evt.mouse.flags |= mouse_flag_mclick;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_MOUSEWHEEL: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_wheel;
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			evt.mouse.w = (u8)(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_XBUTTONDOWN: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_press;
			i16 xbutton = GET_XBUTTON_WPARAM(wParam);
			if(xbutton == XBUTTON1) {
				evt.mouse.flags |= mouse_flag_x1click;
			} else {
				evt.mouse.flags |= mouse_flag_x2click;
			}
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_XBUTTONUP: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_release;
			i16 xbutton = GET_XBUTTON_WPARAM(wParam);
			if(xbutton == XBUTTON1) {
				evt.mouse.flags |= mouse_flag_x1click;
			} else {
				evt.mouse.flags |= mouse_flag_x2click;
			}
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_XBUTTONDBLCLK: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_double;
			i16 xbutton = GET_XBUTTON_WPARAM(wParam);
			if(xbutton == XBUTTON1) {
				evt.mouse.flags |= mouse_flag_x1click;
			} else {
				evt.mouse.flags |= mouse_flag_x2click;
			}
			evt.mouse.x = GET_X_LPARAM(lParam);
			evt.mouse.y = GET_Y_LPARAM(lParam);
			global_enqueue(global_enqueue_param, evt);
			return 0;
		}
		case WM_MOUSEMOVE: {
			evt.type = event_mouse;
			evt.mouse.flags |= mouse_flag_move;
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

#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001

platform_error platform_create_window(platform_window* window, string title, u32 width, u32 height) {

	platform_error ret;

	window->title = make_copy_string(title, &platform_heap_alloc);

	HINSTANCE instance = GetModuleHandleA(NULL);

	if(instance == NULL) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	window->window_class = {
		sizeof(WNDCLASSEX),
		CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS,
		(WNDPROC)window_proc,
		0, 0,
		instance,
		0, 0, 0, 0,
		window->title.c_str, 0
	};

	if(RegisterClassExA(&window->window_class) == 0) {
		ret.good = false;
		ret.error = GetLastError();
		return ret;
	}

	window->handle = CreateWindowExA(WS_EX_ACCEPTFILES, window->title.c_str, window->title.c_str, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			           				 CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, instance, 0);

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

	free_string(window->title, &platform_heap_free);

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