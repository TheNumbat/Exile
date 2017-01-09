
#include "state.hpp"

game_state::game_state() {
	running = false;
	initialized = false;
}

game_state::~game_state() {
	kill();
}

void game_state::init() {

	glfw.init();
	gl.init();

	input_state state("default");
	state.key_button = [this](int key, int, int, int) {
		if(key == GLFW_KEY_ESCAPE) {
			glfw.close_window();
		}
	};
	state.cursor_pos = [this](double x, double y) -> void {
		static double mx = x, my = y;
		gl.cam.move(x - mx, y - my);
		mx = x; my = y;
	};
	state.every_frame = [this]() -> void {
		if(glfw.keydown(GLFW_KEY_W)) {
			gl.cam.pos += gl.cam.front * gl.cam.speed;
		}
		else if(glfw.keydown(GLFW_KEY_S)) {
			gl.cam.pos -= gl.cam.front * gl.cam.speed;
		}
		else if(glfw.keydown(GLFW_KEY_D)) {
			gl.cam.pos += gl.cam.right * gl.cam.speed;
		}
		else if(glfw.keydown(GLFW_KEY_A)) {
			gl.cam.pos -= gl.cam.right * gl.cam.speed;
		}
	};
	state.close_window = [this]() -> void {running = false;};
	glfw.input_add_state(state);

	running = true;
	initialized = true;
}

void game_state::kill() {
	if(initialized) {
		gl.kill();
	    glfw.kill();
		initialized = false;
	}
}

void game_state::run() {
	while (running) {
		glfw.events();

		gl.clear_frame();
		gl.render_box();

		glfw.swap_window();
	}
}
