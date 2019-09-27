
#pragma once

#include <SDL2/SDL.h>
#include "lib/basic.h"

struct Platform {

    void init();
    void destroy();

    void gl_validate();

    SDL_Window* window = null;
    SDL_GLContext gl_context = null;
};
