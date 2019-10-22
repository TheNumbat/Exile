
#include "lib/lib.h"
#include "engine.h"

// This should run before the atexit in static_Varena initializers
static i32 __memcheck = atexit(mem_validate);

i32 main(i32 argc, char **argv) {

    Engine eng;
    eng.init();
    eng.loop();
    eng.destroy();

    return 0;
}