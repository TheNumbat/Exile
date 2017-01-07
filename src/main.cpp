
#include "window_manager/window_manager.hpp"

int main(int, char**) {

    window_manager wm;

    wm.init();

	while (!wm.shouldClose()) {
		glfwPollEvents();
		wm.swap();
	}

    wm.kill();

    return 0;
}
