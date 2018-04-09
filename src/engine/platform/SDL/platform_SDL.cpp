
#include "platform_SDL.h"

// the SDL platform layer can use SDL + standard libraries; the windows layer can only use Win32
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <time.h>

#ifdef _WIN32
extern "C" {
    // Request dGPU
    __declspec(dllexport) bool NvOptimusEnablement = true;
    __declspec(dllexport) bool AmdPowerXpressRequestHighPerformance = true;
}
#endif

#define UNIMPLEMENTED assert(!"UNIMPLEMENTED");

void (*global_enqueue)(void* queue_param, platform_event evt) = null;
void* global_enqueue_param = null;

void platform_test_api() {

#define CHECK_ERR if(!err.good) {printf("ERROR: %s", err.error_message.c_str); exit(1);}

	platform_error err;

	puts("Hello World!");

	printf("log cpus: %d\n", sdl_get_num_cpus());
	printf("phy cpus: %d\n", sdl_get_phys_cpus());

	printf("perfc: %llu\n", sdl_get_perfcount());
	printf("perff: %llu\n", sdl_get_perfcount_freq());

	u64 val = 0;
	printf("val: %llu\n", val);
	sdl_atomic_exchange(&val, 10);
	printf("val: %llu\n", val);

	printf("count: %d\n", global_num_allocs);
	string path;
	err = sdl_get_bin_path(&path);
	CHECK_ERR
	printf("count: %d\n", global_num_allocs);
	puts(path.c_str);
#ifdef TEST_NET_ZERO_ALLOCS
	free_string(path, sdl_heap_free_net);
#else
	free_string(path, sdl_heap_free);
#endif
	printf("count: %d\n", global_num_allocs);

	string time = sdl_time_string();
	printf("%s\n", time.c_str);
	sdl_heap_free(time.c_str);

	fflush(stdout);

#undef CHECK_ERR
}

platform_api platform_build_api() {

	platform_api ret;

	ret.get_perfcount			= &sdl_get_perfcount;
	ret.get_perfcount_freq		= &sdl_get_perfcount_freq;
	ret.create_window			= &sdl_create_window;
	ret.destroy_window			= &sdl_destroy_window;
	ret.swap_buffers			= &sdl_swap_buffers;
	ret.set_queue_callback		= &sdl_set_queue_callback;
	ret.pump_events				= &sdl_pump_events;
	ret.queue_event 			= &sdl_queue_event;
	ret.wait_message			= &sdl_wait_message;
	ret.load_library			= &sdl_load_library;
	ret.free_library			= &sdl_free_library;
	ret.get_proc_address 		= &sdl_get_proc_address;
	ret.get_file_attributes 	= &sdl_get_file_attributes;
	ret.test_file_written		= &sdl_test_file_written;
	ret.copy_file				= &sdl_copy_file;
#ifdef TEST_NET_ZERO_ALLOCS
	ret.heap_alloc				= &sdl_heap_alloc_net;
	ret.heap_free				= &sdl_heap_free_net;
#else
	ret.heap_alloc				= &sdl_heap_alloc;
	ret.heap_free				= &sdl_heap_free;
#endif
	ret.heap_realloc			= &sdl_heap_realloc;
	ret.get_bin_path			= &sdl_get_bin_path;
	ret.create_thread			= &sdl_create_thread;
	ret.this_thread_id			= &sdl_this_thread_id;
	ret.thread_sleep			= &sdl_thread_sleep;
	ret.create_semaphore		= &sdl_create_semaphore;
	ret.destroy_semaphore		= &sdl_destroy_semaphore;
	ret.signal_semaphore		= &sdl_signal_semaphore;
	ret.wait_semaphore			= &sdl_wait_semaphore;
	ret.create_mutex			= &sdl_create_mutex;
	ret.destroy_mutex			= &sdl_destroy_mutex;
	ret.aquire_mutex			= &sdl_aquire_mutex;
	ret.try_aquire_mutex		= &sdl_try_aquire_mutex;
	ret.release_mutex			= &sdl_release_mutex;
	ret.destroy_thread			= &sdl_destroy_thread;
	ret.get_num_cpus			= &sdl_get_num_cpus;
	ret.join_thread				= &sdl_join_thread;
	ret.create_file				= &sdl_create_file;
	ret.close_file				= &sdl_close_file;
	ret.write_file				= &sdl_write_file;
	ret.read_file				= &sdl_read_file;
	ret.get_stdout_as_file		= &sdl_get_stdout_as_file;
	ret.time_string				= &sdl_time_string;
	ret.get_window_size			= &sdl_get_window_size;
	ret.write_stdout			= &sdl_write_stdout;
	ret.file_size				= &sdl_file_size;
	ret.get_glproc				= &sdl_get_glproc;
	ret.keydown					= &sdl_keydown;
	ret.is_debugging			= &sdl_is_debugging;
	ret.debug_break				= &sdl_debug_break;
	ret.set_cursor				= &sdl_set_cursor;
	ret.this_dll				= &sdl_this_dll;
	ret.capture_mouse			= &sdl_capture_mouse;
	ret.release_mouse			= &sdl_release_mouse;
	ret.set_cursor_pos			= &sdl_set_cursor_pos;
	ret.atomic_exchange 		= &sdl_atomic_exchange;
	ret.window_focused 			= &sdl_window_focused;
	ret.get_phys_cpus			= &sdl_get_phys_cpus;

	return ret;
}

i32 sdl_get_phys_cpus() {
	
	i32 cpus = sdl_get_num_cpus();
	bool HT = false;

#ifdef _MSC_VER
	i32 cpuinfo[4];
	__cpuid(cpuinfo, 1);
	
	HT = (cpuinfo[3] & (1 << 28)) > 0;
#else
#error Fix this
#endif

	return HT ? cpus / 2 : cpus;
}

u64 sdl_atomic_exchange(u64* dest, u64 val) {

	return (u64)SDL_AtomicSetPtr((void**)dest, (void*)val);
}

bool sdl_window_focused(platform_window* win) {

	UNIMPLEMENTED;
	return false;
}

platform_error sdl_set_cursor_pos(platform_window* win, i32 x, i32 y) {

	platform_error ret;

	UNIMPLEMENTED;
	return ret;	
}

void sdl_capture_mouse(platform_window* win) {

	SDL_CaptureMouse(SDL_TRUE);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_ShowCursor(SDL_FALSE);
}

void sdl_release_mouse() {

	SDL_CaptureMouse(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_FALSE);
	SDL_ShowCursor(SDL_TRUE);	
}

u64 sdl_get_perfcount() {

	return SDL_GetPerformanceCounter();
}

u64 sdl_get_perfcount_freq() {

	return SDL_GetPerformanceFrequency();
}

platform_error sdl_this_dll(platform_dll* dll) {

	platform_error ret;

	UNIMPLEMENTED;
	return ret;
}

// no good platform-independent way to do this...could be done but it's not worth it
void sdl_debug_break() {
#ifdef MSC_VER
	__debugbreak();
#endif
}
bool sdl_is_debugging() {
	return false;
}

void sdl_set_cursor(cursors c) {

	UNIMPLEMENTED;
}

platform_error sdl_create_window(platform_window* window, string title, u32 width, u32 height) {

	platform_error ret;

	window->title = title;
	window->w = width;
	window->h = height;

	if(SDL_Init(SDL_INIT_VIDEO)) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
	}

	window->window = SDL_CreateWindow(title.c_str, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	
	if(!window->window) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#define TRY_VERSION(MAJOR, MINOR) SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, MAJOR); \
								  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, MINOR); \
								  if(window->gl_context == null) window->gl_context = SDL_GL_CreateContext(window->window);

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
		ret.error_message = str(SDL_GetError());
	}

	// TODO(max): vsync/fullscreen/AA/etc settings
	SDL_GL_SetSwapInterval(0);

	return ret;
}

platform_error sdl_destroy_window(platform_window* window) {

	platform_error ret;

	UNIMPLEMENTED;
	return ret;
}

platform_error sdl_get_window_size(platform_window* window, i32* w, i32* h) {

	platform_error ret;

	UNIMPLEMENTED;
	return ret;
}

platform_error sdl_swap_buffers(platform_window* window) {

	platform_error ret;

	UNIMPLEMENTED;
	return ret;
}

void sdl_set_queue_callback(void (*enqueue)(void* queue_param, platform_event evt), void* queue_param) {

	global_enqueue = enqueue;
	global_enqueue_param = queue_param;
}

void sdl_pump_events(platform_window* window) {

	UNIMPLEMENTED;
}

void sdl_queue_event(platform_event evt) {

	global_enqueue(global_enqueue_param, evt);
}

platform_error sdl_wait_message() {

	platform_error ret;

	UNIMPLEMENTED;
	return ret;
}

bool sdl_keydown(platform_keycode key) {

	UNIMPLEMENTED;
	return false;
}

platform_error sdl_load_library(platform_dll* dll, string file_path) {

	platform_error ret;

	dll->handle = SDL_LoadObject(file_path.c_str);
	
	if(!dll->handle) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
	}

	return ret;
}

platform_error sdl_free_library(platform_dll* dll) {

	platform_error ret;

	if(dll->handle) {
		SDL_UnloadObject(dll->handle);
		dll->handle = null;
	}

	return ret;
}

platform_error sdl_get_proc_address(void** address, platform_dll* dll, string name) {

	platform_error ret;

	*address = SDL_LoadFunction(dll->handle, name.c_str);

	if(!*address) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
	}

	return ret;
}

void* sdl_get_glproc(string name) {

	return SDL_GL_GetProcAddress(name.c_str);
}

platform_error sdl_copy_file(string source, string dest, bool overwrite) {

	platform_error ret;

	platform_file d, s;
	
	// this is why defer {} is good...

	ret = sdl_create_file(&s, source, platform_file_open_op::existing);
	if(!ret.good) goto close;

	ret = sdl_create_file(&d, dest, platform_file_open_op::cleared);
	if(!ret.good) goto close;

	u32 s_size = sdl_file_size(&s);

	void* contents = sdl_heap_alloc(s_size);

	ret = sdl_read_file(&s, contents, s_size);
	if(!ret.good) goto free_close;

	ret = sdl_write_file(&d, contents, s_size);

	free_close:

	sdl_heap_free(contents);

	close:

	sdl_close_file(&s);
	sdl_close_file(&d);
	return ret;
}

platform_error sdl_get_file_attributes(platform_file_attributes* attrib, string file_path) {

	platform_error ret;

	if(stat(file_path.c_str, &attrib->info)) {
		ret.good = false;
		ret.error = errno;
	}
	
	return ret;
}

bool sdl_test_file_written(platform_file_attributes* first, platform_file_attributes* second) {

	return second->info.st_mtime > first->info.st_mtime;
}

platform_error sdl_create_file(platform_file* file, string path, platform_file_open_op mode) {

	platform_error ret;

	const char* creation = null;

	switch(mode) {
		case platform_file_open_op::existing:
		creation = "r+b";
		break;
		case platform_file_open_op::cleared:
		creation = "w+b";
		break;
		default:
		ret.good = false;
		return ret;	
		break;
	}

	file->ops = SDL_RWFromFile(path.c_str, creation);

	if(file->ops == null) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
		return ret;		
	}

	file->path = path;
	return ret;
}

platform_error sdl_close_file(platform_file* file) {

	platform_error ret;

	if(SDL_RWclose(file->ops)) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
	}

	file->ops = null;

	return ret;
}

platform_error sdl_write_file(platform_file* file, void* mem, u32 bytes) {

	platform_error ret;

	if(SDL_RWwrite(file->ops, mem, 1, bytes) < bytes) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
	}

	return ret;
}

platform_error sdl_read_file(platform_file* file, void* mem, u32 bytes) {

	platform_error ret;

	if(SDL_RWread(file->ops, mem, 1, bytes) < bytes) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
	}

	return ret;
}

u32	sdl_file_size(platform_file* file) {

	i64 current = SDL_RWseek(file->ops, 0, RW_SEEK_CUR);
	SDL_RWseek(file->ops, 0, RW_SEEK_END);
	i64 size = SDL_RWtell(file->ops);
	SDL_RWseek(file->ops, current, RW_SEEK_SET);
	
	return (u32)size;
}

platform_error sdl_get_stdout_as_file(platform_file* file) {

	platform_error ret;

	file->ops = SDL_RWFromFP(stdout, SDL_FALSE);
	if(!file->ops) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
	}

	return ret;
}

platform_error sdl_write_stdout(string str) {

	platform_error ret;
	puts(str.c_str);
	return ret;
}

void* sdl_heap_alloc(u64 bytes) {

	return malloc(bytes);
}

void* sdl_heap_realloc(void* mem, u64 bytes) {

	return realloc(mem, bytes);
}

void sdl_heap_free(void* mem) {

	free(mem);
}

void* sdl_heap_alloc_net(u64 bytes) {

	SDL_AtomicAdd((SDL_atomic_t*)&global_num_allocs, 1);
	return malloc(bytes);
}

void sdl_heap_free_net(void* mem) {

	SDL_AtomicAdd((SDL_atomic_t*)&global_num_allocs, -1);
	return free(mem);
}

platform_error sdl_get_bin_path(string* p) {

	platform_error ret;

	char* path = SDL_GetBasePath();

	if(!path) {

		ret.good = false;
		ret.error_message = str(SDL_GetError());

	} else {

#ifdef TEST_NET_ZERO_ALLOCS
		*p = make_string_from_c_str(path, sdl_heap_alloc_net);
#else
		*p = make_string_from_c_str(path, sdl_heap_alloc);
#endif
	}
	
	return ret;
}

platform_error sdl_create_thread(platform_thread* thread, i32 (*proc)(void*), void* param, bool start_suspended) {

	platform_error ret;

	thread->thrd = SDL_CreateThread(proc, "", param);

	if(!thread->thrd) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
	}

	return ret;
}

platform_thread_join_state sdl_join_thread(platform_thread* thread, i32 ms) {

	platform_thread_join_state ret;

	i32 result = 0;

	SDL_WaitThread(thread->thrd, &result);

	ret.state = _platform_thread_join_state::joined;
	thread->thrd = null;

	return ret;
}

platform_error sdl_destroy_thread(platform_thread* thread) {

	// NOTE(max): sdl_wait_thread destroys the thread

	platform_error ret;
	return ret;
}

platform_thread_id sdl_this_thread_id() {

	return (platform_thread_id)SDL_ThreadID();
}

void sdl_thread_sleep(i32 ms) {

	SDL_Delay((u32)ms);
}

i32 sdl_get_num_cpus() {

	return SDL_GetCPUCount(); 
}

platform_error sdl_create_semaphore(platform_semaphore* sem, i32 initial_count, i32 max_count) {

	platform_error ret;

	sem->sem = SDL_CreateSemaphore(initial_count);
	if(!sem->sem) {
		ret.good = false;
		ret.error_message = str(SDL_GetError());
	}
	
	return ret;
}

platform_error sdl_destroy_semaphore(platform_semaphore* sem) {

	platform_error ret;

	SDL_DestroySemaphore(sem->sem);
	sem->sem = null;

	return ret;
}

platform_semaphore_state sdl_wait_semaphore(platform_semaphore* sem, i32 ms) {

	platform_semaphore_state ret;

	switch(SDL_SemWaitTimeout(sem->sem, (u32)ms)) {
	case 0:
		ret.state = _platform_semaphore_state::signaled;
		break;
	case SDL_MUTEX_TIMEDOUT:
		ret.state = _platform_semaphore_state::timed_out;
		break;
	default:
		ret.state = _platform_semaphore_state::failed;
		ret.error.good = false;
		ret.error.error_message = str(SDL_GetError());
		break;
	}

	return ret;
}

platform_error sdl_signal_semaphore(platform_semaphore* sem, i32 times) {

	platform_error ret;

	for(i32 i = 0; i < times; i++)
		SDL_SemPost(sem->sem);

	return ret;
} 

void sdl_create_mutex(platform_mutex* mut, bool aquire) {

	mut->mut = SDL_CreateMutex();
	
	if(aquire)
		sdl_aquire_mutex(mut);
}

void sdl_destroy_mutex(platform_mutex* mut) {

	SDL_DestroyMutex(mut->mut);
	mut->mut = null;
}

void sdl_aquire_mutex(platform_mutex* mut) {

	SDL_LockMutex(mut->mut);
}

bool sdl_try_aquire_mutex(platform_mutex* mut) {

	return SDL_TryLockMutex(mut->mut) == 0;
}

void sdl_release_mutex(platform_mutex* mut) {

	SDL_UnlockMutex(mut->mut);
}

string sdl_time_string() {

	time_t raw = time(null);
	struct tm info;
	localtime_s(&info, &raw);

#ifdef TEST_NET_ZERO_ALLOCS
	string ret = make_string(9, &sdl_heap_alloc_net);
#else
	string ret = make_string(9, &sdl_heap_alloc);
#endif

	strftime(ret.c_str, 9, "%I:%M:%S", &info);
	
	return ret;
}
