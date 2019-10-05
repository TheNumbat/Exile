
#ifndef RUNNING_META
#include <imgui/imgui.h>
#endif

#include "engine.h"

void Engine::init() {
    plt.init();
    imgui.init(plt);
}

void Engine::loop() {

    bool running = true;
    while(running) {
    
        imgui.begin_frame(plt);
        //ImGuiIO& io = ImGui::GetIO(); sorry lol the warning is annoying :)

        SDL_Event e;
        while(imgui.get_event(e)) {

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

        imgui.end_frame(plt);
    }
}

void Engine::destroy() {
    imgui.destroy();
    plt.destroy();
}
