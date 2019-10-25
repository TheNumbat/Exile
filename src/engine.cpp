
#include "engine.h"
#include "profiler.h"

void Engine::init() {
    Profiler::start_thread();
    wnd.init();
}

void Engine::loop() {

    bool running = true;
    while(running) {
        Profiler::begin_frame();
    
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
        Profiler::end_frame();
    }
}

void Engine::destroy() {
    wnd.destroy();
    Profiler::end_thread();
    Profiler::destroy();
}
