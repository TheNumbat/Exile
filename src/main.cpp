
#include "common.hpp"
#include "window_manager/window_manager.hpp"

int main(int, char**) {

	LOG_BEGIN_THIS_THREAD();
	LOG_PUSH_CONTEXT(main);

	LOG_INFO("lmao");

    window_manager wm;

    wm.init();

	while (!wm.shouldClose()) {
		glfwWaitEvents();
		wm.swap();
	}

    wm.kill();

    LOG_POP_CONTEXT();

    return 0;
}
