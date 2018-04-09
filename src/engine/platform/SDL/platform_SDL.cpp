
#include "platform_SDL.h"

// the SDL platform layer can use SDL + standard libraries; the windows layer can only use Win32
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

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
	ret.terminate_thread		= &sdl_terminate_thread;
	ret.exit_this_thread		= &sdl_exit_this_thread;
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
	ret.get_timef				= &sdl_get_timef;
	ret.make_timef				= &sdl_make_timef;
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

	UNIMPLEMENTED;
}

void sdl_release_mouse() {

	UNIMPLEMENTED;
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
void sdl_debug_break() {}
bool sdl_is_debugging() {
	return false;
}

void sdl_set_cursor(cursors c) {

	UNIMPLEMENTED;
}

platform_error sdl_create_window(platform_window* window, string title, u32 width, u32 height) {

	platform_error ret;

	UNIMPLEMENTED;
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

	UNIMPLEMENTED;
	return ret;
}

platform_thread_join_state sdl_join_thread(platform_thread* thread, i32 ms) {

	platform_thread_join_state ret;

	i32 result = 0;

	SDL_WaitThread(thread->thrd, &result);

	ret.state = _platform_thread_join_state::joined;

	return ret;
}

platform_error sdl_destroy_thread(platform_thread* thread) {

	platform_error ret;

	UNIMPLEMENTED;
	return ret;
}

platform_thread_id sdl_this_thread_id() {

	return (platform_thread_id)SDL_ThreadID();
}

platform_error sdl_terminate_thread(platform_thread* thread, i32 exit_code) {

	platform_error ret;

	UNIMPLEMENTED;
	return ret;
}

void sdl_exit_this_thread(i32 exit_code) {

	UNIMPLEMENTED;
}

void sdl_thread_sleep(i32 ms) {

	UNIMPLEMENTED;
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

string sdl_make_timef(string fmt) {

	string ret;

	UNIMPLEMENTED;
	return ret;
}

void sdl_get_timef(string fmt, string* out) {

	UNIMPLEMENTED;
}
