
#pragma once

#include "window.h"

struct Engine {

    void init();
    void loop();
    void destroy();

private:
    Window wnd;
};
