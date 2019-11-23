
#pragma once

#include "window.h"
#include "gui/dbg_gui.h"

struct Engine {

    void init();
    void loop();
    void destroy();

private:
    Window wnd;
    Dbg_Gui dbg_gui;
};
