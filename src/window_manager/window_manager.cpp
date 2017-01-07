
#include "window_manager.hpp"

#include <iostream>

window_manager::window_manager() {
	window = nullptr;
	initialized = false;
}

window_manager::~window_manager() {
	kill();
}

void window_manager::init() {
	initialized = true;
	glfwInit();
	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);

	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	window = glfwCreateWindow(1280, 720, "CaveGame", nullptr, nullptr);

	glfwMakeContextCurrent(window);
}

void window_manager::kill() {
	if(initialized) {
		glfwDestroyWindow(window);
		glfwTerminate();
		initialized = false;
	}
}

void window_manager::error_callback(int err, const char* what) {
	std::cout << "GLFW Error #" << err << ": " << what << std::endl;
}

bool window_manager::shouldClose() {
	return !!glfwWindowShouldClose(window);
}

void window_manager::swap() {
	glfwSwapBuffers(window);
}

void window_manager::size(int* w, int* h) {
	glfwGetWindowSize(window, w, h);
}

void window_manager::resize(int w, int h) {
	glfwSetWindowSize(window, w, h);
}