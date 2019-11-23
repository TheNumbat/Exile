
#pragma once

#include "../lib/lib.h"
#include "profiler_gui.h"

struct Dbg_Gui {

    void init();
    void destroy();

    void profiler();

    Profiler_Window prof_window;
};
