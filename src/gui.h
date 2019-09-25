
#pragma once

#include "platform.h"

struct Gui {

    void init(const Platform& plt);
    void destroy();

    void begin_frame(const Platform& plt);
    void end_frame(const Platform& plt);

    bool get_event(SDL_Event& e);
};
