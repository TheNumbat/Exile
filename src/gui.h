
#pragma once

#include "platform.h"
#include "lib/lib.h"

struct Gui {

    static constexpr char a_name[] = "imgui";
    using alloc = Mallocator<a_name>;

    void init(const Platform& plt);
    void destroy();

    void begin_frame(const Platform& plt);
    void end_frame(const Platform& plt);

    bool get_event(SDL_Event& e);
};
