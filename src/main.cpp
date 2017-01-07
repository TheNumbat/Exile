
#include "common.hpp"
#include "window_manager/window_manager.hpp"
#include "gl/gl.hpp"

int main(int, char**) {

	LOG_BEGIN_THIS_THREAD();
	LOG_PUSH_CONTEXT(MAIN);

	LOG_INFO("STARTED MAIN");

    window_manager wm;
	gl_manager gl;

	wm.init();
	gl.init();

    LOG_INFO("Done with initialization!");
    LOG_INFO("...");

	while (!wm.shouldClose()) {
		glfwPollEvents();
		gl.clear_frame();
		gl.render_box();
		wm.swap();
	}

	LOG_INFO("Shutting down...");

	gl.kill();
    wm.kill();

    LOG_POP_CONTEXT();

    return 0;
}
