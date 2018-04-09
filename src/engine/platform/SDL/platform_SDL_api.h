
#include <SDL2/SDL.h>

/* Included in platform_api.h */

u32 PLATFORM_SHARING_ERROR = UINT32_MAX;

struct platform_window {
	// Transparent
	string title;
	i32 w = 0, h = 0;

	// Opaque block, do not use outside of platform code
	SDL_Window* window = null;
};

struct platform_dll {
	// Opaque
	void* handle = null;
};

struct platform_file_attributes {
	// Opaque
	
};

struct platform_thread {
	// transparent
	platform_thread_id id;

	// Opaque
	
};

struct platform_semaphore {
	// Opaque
	
};

struct platform_mutex {
	// Opaque
	
};

struct platform_file {
	// Transparent
	string path;

	// Opaque
	SDL_RWops* ops = null;
};

bool operator==(platform_file first, platform_file second) {
	return false;
}
