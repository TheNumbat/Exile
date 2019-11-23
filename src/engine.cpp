
#include "engine.h"
#include "gui/dbg_gui.h"

void Engine::init() {
    Profiler::start_thread();
    wnd.init();
}

void Engine::loop() {

    bool running = true;
    while(running) {
        Profiler::begin_frame();
    
        SDL_Event e;
        Profiler::enter("Events");
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

        DbgGui::Profiler();

        wnd.end_frame();

        Profiler::end_frame();

        Mframe::reset();
    }
}

void Engine::destroy() {
    wnd.destroy();
    Profiler::end_thread();
}
