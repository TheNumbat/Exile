
#pragma once

#include <SDL2/SDL.h>
#include "lib/lib.h"

struct Window {

    void init();
    void destroy();

    void begin_frame();
    void end_frame();
    
    bool get_event(SDL_Event& e);

    static constexpr char gui_name[] = "ImGui";
    using gui_alloc = Mallocator<gui_name>;

private:
    void gl_validate();

    SDL_Window* window = null;
    SDL_GLContext gl_context = null;
};
