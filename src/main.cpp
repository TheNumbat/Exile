
#include "common.hpp"
#include "window_manager/window_manager.hpp"

int main(int, char**) {

	LOG_BEGIN_THIS_THREAD();
	LOG_PUSH_CONTEXT(MAIN);

	LOG_INFO("STARTED MAIN");

    window_manager wm;

    wm.init();

    LOG_INFO("Done with initialization!");
    LOG_INFO("...");

	while (!wm.shouldClose()) {
		glfwWaitEvents();
		wm.swap();
	}

	LOG_INFO("Shutting down...");

    wm.kill();

    LOG_POP_CONTEXT();

    return 0;
}
