
#include "engine.h"
#include "gui/dbg_gui.h"

void Engine::init() {
    Profiler::start_thread();
    wnd.init();
    dbg_gui.init();
}

void Engine::loop() {

    bool running = true;
    while(running) {
        Profiler::begin_frame();
    
        // actual event handling system
        Profiler::enter("Events");
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
        Profiler::exit();

        wnd.begin_frame();

        // actual frame stuff

        dbg_gui.profiler();
        wnd.end_frame();
        Profiler::end_frame();
        Mframe::reset();
    }
}

void Engine::destroy() {
    wnd.destroy();
    dbg_gui.destroy();
    Profiler::end_thread();
}
