
#include "common.hpp"
#include "glfw_manager/glfw_manager.hpp"
#include "gl/gl.hpp"

void key(int key, int, int action, int) {
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		LOG_INFO("ESC");
	}
}

int main(int, char**) {

	LOG_BEGIN_THIS_THREAD();
	LOG_PUSH_CONTEXT(MAIN);

	LOG_INFO("STARTED MAIN");

    glfw_manager glfw;
	gl_manager gl;

	glfw.init();
	gl.init();

	input_state state("default");
	state.key_button = key;
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
