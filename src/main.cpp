
#include "common.hpp"
#include "glfw_manager/glfw_manager.hpp"
#include "gl/gl.hpp"

int main(int, char**) {

	LOG_BEGIN_THIS_THREAD();
	LOG_PUSH_CONTEXT(MAIN);

	LOG_INFO("STARTED MAIN");

    glfw_manager glfw;
	gl_manager gl;

	glfw.init();
	gl.init();

	input_state state("default");
	state.key_button = [&glfw](int key, int, int, int) {
		if(key == GLFW_KEY_ESCAPE) {
			glfw.close_window();
		}
	};
	state.cursor_pos = [&gl](double x, double y) -> void {
		static double mx = x, my = y;
		gl.cam.move(x - mx, y - my);
		mx = x; my = y;
	};
	state.every_frame = [&gl, &glfw]() -> void {
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
	glfw.input_add_state(state);

    LOG_INFO("Done with initialization!");
    LOG_INFO("...");

	while (!glfw.window_should_close()) {
		glfw.events();

		gl.clear_frame();
		gl.render_box();

		glfw.swap_window();
	}

	LOG_INFO("Shutting down...");

	gl.kill();
    glfw.kill();

    LOG_POP_CONTEXT();

    return 0;
}
