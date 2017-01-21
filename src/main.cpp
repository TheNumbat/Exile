
#include "state.hpp"

int main(int, char**) {
	LOG_BEGIN_THIS_THREAD(MAIN);
	LOG_PUSH_CONTEXT(MAIN);

	LOG_INFO("STARTED MAIN");

	game_state state;

	state.init();

    LOG_INFO("Done with initialization!");

    gdebug.print_tree();

	state.run();

	LOG_INFO("Shutting down...");
    
    state.kill();

    LOG_POP_CONTEXT();

    return 0;
}
