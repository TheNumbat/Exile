
#include "../common.hpp"
#include "glfw_manager.hpp"

#include <iostream>

static std::string input_current_state = "";
static std::map<std::string, input_state> input_states;

void errorfun(int err, const char* what) {
	LOG_ERROR("GLFW Error #" + std::to_string(err) +  ": " + what);
}

void mousebuttonfun(GLFWwindow*, int button, int action, int mods) {
	if(input_current_state.size()) {
		auto state_entry = input_states.find(input_current_state);
		assert(state_entry != input_states.end());
		state_entry->second.mouse_button(button, action, mods);
	}
}

void cursorposfun(GLFWwindow*, double x, double y) {
	if(input_current_state.size()) {
		auto state_entry = input_states.find(input_current_state);
		assert(state_entry != input_states.end());
		state_entry->second.cursor_pos(x, y);
	}
}

void cursorenterfun(GLFWwindow*, int entered) {
	if(input_current_state.size()) {
		auto state_entry = input_states.find(input_current_state);
		assert(state_entry != input_states.end());
		state_entry->second.cursor_enter(entered);
	}
}

void scrollfun(GLFWwindow*, double x, double y) {
	if(input_current_state.size()) {
		auto state_entry = input_states.find(input_current_state);
		assert(state_entry != input_states.end());
		state_entry->second.mouse_scroll(x, y);
	}
}

void keyfun(GLFWwindow*, int key, int scan, int action, int mods) {
	if(input_current_state.size()) {
		auto state_entry = input_states.find(input_current_state);
		assert(state_entry != input_states.end());
		state_entry->second.key_button(key, scan, action, mods);
	}
}

void dropfun(GLFWwindow*, int count, const char** paths) {
	if(input_current_state.size()) {
		auto state_entry = input_states.find(input_current_state);
		assert(state_entry != input_states.end());
		std::vector<std::string> vec;
		for(int i = 0; i < count; i++)
			vec.push_back(std::string(paths[i]));
		state_entry->second.file_drop(vec);
	}
}

void closefun(GLFWwindow*) {
	if(input_current_state.size()) {
		auto state_entry = input_states.find(input_current_state);
		assert(state_entry != input_states.end());
		state_entry->second.close_window();
	}
}

input_state::input_state(std::string name) {
	state = name;
	mouse_button = [](int,int,int) -> void {};
	cursor_pos = [](double, double) -> void {};
	cursor_enter = [](int) -> void {};
	mouse_scroll = [](double, double) -> void {};
	key_button = [](int,int,int,int) -> void {};
	file_drop = [](std::vector<std::string>) -> void {};
	every_frame = []() -> void {};
	close_window = []() -> void {};
}

glfw_manager::glfw_manager() {
	window = nullptr;
	initialized = false;
}

glfw_manager::~glfw_manager() {
	kill();
}

void glfw_manager::events() {
	glfwPollEvents();
	if(input_current_state.size()) {
		auto state_entry = input_states.find(input_current_state);
		assert(state_entry != input_states.end());
		state_entry->second.every_frame();
	}
}

void glfw_manager::init() {
	LOG_PUSH_CONTEXT(WINDOW);
	LOG_INFO("Initializing Window System");
	LOG_PUSH_SEC();

	if(!glfwInit()) {
		LOG_FATAL("Failed to initialize GLFW!");
		return;
	}
	LOG_INFO("Initializing GLFW");
	
	glfwSetErrorCallback(&errorfun);

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

 	glfwSetKeyCallback(window, &keyfun);
 	glfwSetMouseButtonCallback(window, &mousebuttonfun);
 	glfwSetCursorPosCallback(window, &cursorposfun);
 	glfwSetCursorEnterCallback(window, &cursorenterfun);
 	glfwSetScrollCallback(window, &scrollfun);
 	glfwSetDropCallback(window, &dropfun);
 	glfwSetWindowCloseCallback(window, &closefun);

 	LOG_INFO("Set GLFW input callbacks");

	glfwMakeContextCurrent(window);
	glfwSwapInterval(-1);

	LOG_INFO("Done initializing window system");
	LOG_INFO((std::string)"GLFW: " + glfwGetVersionString());
	LOG_INFO((std::string)"OpenGL: " + (const char*)glGetString(GL_VERSION));
	LOG_INFO((std::string)"Renderer: " + (const char*)glGetString(GL_RENDERER));
	LOG_POP_SEC();
	LOG_POP_CONTEXT();

	initialized = true;
}

void glfw_manager::kill() {
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

bool glfw_manager::keydown(int key) {
	return !!glfwGetKey(window, key);
}

bool glfw_manager::window_should_close() {
	return !!glfwWindowShouldClose(window);
}

void glfw_manager::close_window() {
	glfwSetWindowShouldClose(window, true);
	closefun(window);
}

void glfw_manager::swap_window() {
	glfwSwapBuffers(window);
}

void glfw_manager::window_size(int* w, int* h) {
	glfwGetWindowSize(window, w, h);
}

void glfw_manager::window_resize(int w, int h) {
	glfwSetWindowSize(window, w, h);
}

void glfw_manager::input_set_state(std::string state) {
	input_current_state = state;
	assert(input_states.find(input_current_state) != input_states.end());
}

void glfw_manager::input_add_state(input_state state, bool use) {
	assert(input_states.find(state.state) == input_states.end());	
	input_states.insert({state.state, state});
	if(use) {
		input_set_state(state.state);
	}
}
