
#include "lib/lib.h"
#include "engine.h"

i32 main(i32 argc, char **argv) {

    vec<i32> test;
    test.push(0);
    test.push(2);
    for(i32 i : test) {
        i = 1;
    }
    info("test: %", test);
    for(i32& i : test) {
        i = 1;
    }
    for(const i32& i : test) {
        // i = 1;
    }
    info("test: %", test);

    Engine eng;
    eng.init();
    eng.loop();
    eng.destroy();

    mem_validate();

    return 0;
}
