
#include "lib/basic.h"
#include "lib/alloc.h"
#include "lib/printf.h"
#include "engine.h"

i32 main(i32 argc, char **argv) {

	f32 test0 = 1;
	i32 test1 = 1;
	info("test: %, %", test0, test1);

	Engine eng;
	eng.init();
	eng.loop();
	eng.destroy();

	mem_validate();

	return 0;
}

//test: [1, 2, 3, 4, 5]