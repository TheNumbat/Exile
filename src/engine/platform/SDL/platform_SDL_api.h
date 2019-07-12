
#ifndef __clang__
#include <SDL2/SDL.h>
#endif
#include <sys/stat.h>
#ifdef WIN32
#define stat _stat
#endif

#define PLT_SHARING_ERROR UINT32_MAX

struct platform_window_internal {
	bool focused = false;
	SDL_Window* window = null;
	SDL_GLContext gl_context = {};
};

struct platform_dll {
	// Opaque
	void* handle = null;
};

struct platform_file_attributes {
	// Opaque
	struct stat info = {};
};

struct platform_thread {
	// transparent
	platform_thread_id id;

	// Opaque
	SDL_Thread* thrd = null;
};

struct platform_semaphore {
	// Opaque
	SDL_sem* sem = null;
};

struct platform_mutex {
	// Opaque
	SDL_mutex* mut = null;
};

struct platform_file {
	// Transparent
	string path;

	// Opaque
	SDL_RWops* ops = null;
};
