
#pragma once

#include "platform.h"

struct test0 {
    i32 e = 0;
    f32 f = 1.0f;
};
struct test1 {
    test0 _0;
    // literal s = "yes";
};

struct Gui {

    void init(const Platform& plt);
    void destroy();

    void begin_frame(const Platform& plt);
    void end_frame(const Platform& plt);

    bool get_event(SDL_Event& e);
};
