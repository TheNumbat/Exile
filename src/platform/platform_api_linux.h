/* Included in platform_api.h */

<<<<<<< HEAD
struct platform_window {

};

struct platform_dll {

};

struct platform_file_attributes {

};

struct platform_thread_id {

};

struct platform_thread {

};

struct platform_semaphore {

};

struct platform_mutex {

};

struct platform_file {

};
=======
u32 PLATFORM_SHARING_ERROR = 32;

struct platform_window {
	// Transparent
	string title;
	i32 w = 0, h = 0;

	// Opaque block, do not use outside of platform code
};

struct platform_dll {
	// Opaque
	
};

struct platform_file_attributes {
	// Opaque
	
};

struct platform_thread_id {
	// Transparent
	
};

bool operator==(platform_thread_id first, platform_thread_id second) {
	return false;
}

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
	
};

bool operator==(platform_file first, platform_file second) {
	return false;
}
>>>>>>> 56b680cb6855c002a04dacb314bb662a4cacb4f6
