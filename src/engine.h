
#pragma once

#include "platform.h"
#include "gui.h"

struct Engine {

    void init();
    void loop();
    void destroy();

private:
    Platform plt;
    Gui gui;

    template<typename T> friend struct Type_Info;
};