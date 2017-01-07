
#pragma once

#include <GLFW/glfw3.h>
#include <string>

class window_manager {
public:
	window_manager();
	~window_manager();

	void init();
	void kill();

	void swap();
	bool shouldClose();

	void size(int* w, int* h);
	void resize(int w, int h);

private:
	static void error_callback(int err, const char* what);

	GLFWwindow* window;
	bool initialized;
};
