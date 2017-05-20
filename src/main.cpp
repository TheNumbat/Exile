
#include <iostream>

#include "common.h"
#include "platform.h"

using std::cout;
using std::endl;

int main() {

	platform_window window = {};
	platform_error err = platform_create_window(&window, "Window", 1280, 720);

	if(!err.good) {
		cout << "Error creating window: " << err.error << endl;
	}

	const char* version  = (const char*)glGetString(GL_VERSION);
	const char* renderer = (const char*)glGetString(GL_RENDERER);
	const char* vendor   = (const char*)glGetString(GL_VENDOR);

	cout << "Vendor  : " << vendor << endl;
	cout << "Version : " << version << endl;
	cout << "Renderer: " << renderer << endl;

	while(platform_process_messages(&window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		platform_swap_buffers(&window);
		platform_wait_message();
	}

	err = platform_destroy_window(&window);

	if(!err.good) {
		cout << "Error destroying window: " << err.error << endl;
	}

	return 0;
}
