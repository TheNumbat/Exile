
#pragma once

#include "../common.hpp"

#include <GLFW/glfw3.h>
#include <string>
#include <map>
#include <functional>

struct input_state {
	input_state(std::string name);
	std::string state;
	std::function<void(int,int,int)> 		mouse_button;
	std::function<void(double, double)>		cursor_pos;
	std::function<void(int)>				cursor_enter;
	std::function<void(double, double)>		mouse_scroll;
	std::function<void(int,int,int,int)>	key_button;
	std::function<void(int,const char **)>	file_drop;
	std::function<void()>					every_frame;
};

class glfw_manager {
public:
	glfw_manager();
	~glfw_manager();

	void init();
	void kill();

	void swap_window();
	bool window_should_close();
	void close_window();

	void events();
	void input_set_state(std::string state);
	void input_add_state(input_state state, bool use = true);
	bool keydown(int key);

	void window_size(int* w, int* h);
	void window_resize(int w, int h);

private:
	GLFWwindow* window;
	bool initialized;
};
