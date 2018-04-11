
#include <SDL2/SDL.h>
#include <sys/stat.h>
#ifdef WIN32
#define stat _stat
#endif

/* Included in platform_api.h */

u32 PLATFORM_SHARING_ERROR = UINT32_MAX;

struct platform_window {
	// Transparent
	string title;
	i32 w = 0, h = 0;

	// Opaque block, do not use outside of platform code
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

bool operator==(platform_file first, platform_file second) {
	return first.ops == second.ops;
}
