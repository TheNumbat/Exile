
#include "engine.h"

void Engine::init() {
    wnd.init();
}

void Engine::loop() {

    bool running = true;
    while(running) {
    
        SDL_Event e;
        while(wnd.get_event(e)) {

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

        wnd.begin_frame();

        wnd.end_frame();
    }
}

void Engine::destroy() {
    wnd.destroy();
}
