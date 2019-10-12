
#ifndef RUNNING_META
#include <imgui/imgui.h>
#endif

#include "engine.h"

void Engine::init() {
    plt.init();
    gui.init(plt);
}

void Engine::loop() {

    bool running = true;
    while(running) {
    
        SDL_Event e;
        while(gui.get_event(e)) {

            switch(e.type) {
            case SDL_QUIT: {
                running = false;
            } break;
            case SDL_KEYDOWN: {
                if(e.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            } break;
            }
        }

        gui.begin_frame(plt);

        gui.end_frame(plt);
    }
}

void Engine::destroy() {
    gui.destroy();
    plt.destroy();
}
