
#include "..\common.hpp"
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
	LOG_PUSH_CONTEXT(WINDOW);
	LOG_INFO("Initializing Window System");
	LOG_PUSH_SEC();

	if(!glfwInit()) {
		LOG_FATAL("Failed to initialize GLFW!");
		return;
	}
	LOG_INFO("Initialized GLFW");
	
	glfwSetErrorCallback(error_callback);
	LOG_INFO("Set GLFW error callback");

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
	if(!window) {
		LOG_FATAL("Failed to create GLFW Window!");
		return;
	}
	LOG_INFO("Created GLFW Window");

	glfwMakeContextCurrent(window);
	glfwSwapInterval(-1);

	LOG_INFO("Done initializing window system");
	LOG_POP_SEC();
	LOG_POP_CONTEXT();

	initialized = true;
}

void window_manager::kill() {
	if(initialized) {
		LOG_PUSH_CONTEXT(WINDOW);
		LOG_INFO("Destroying Window System");
		LOG_PUSH_SEC();

		glfwDestroyWindow(window);
		LOG_INFO("Destroyed GLFW Window");
		glfwTerminate();
		LOG_INFO("Terminated GLFW");

		LOG_POP_SEC();
		LOG_POP_CONTEXT();

		initialized = false;
	}
}

void window_manager::error_callback(int err, const char* what) {
	LOG_ERROR("GLFW Error #" + std::to_string(err) +  ": " + what);
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