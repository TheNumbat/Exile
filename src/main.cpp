
#include "lib/basic.h"
#include "lib/alloc.h"
#include "lib/reflect.h"
#include "engine.h"

i32 main(i32 argc, char **argv) {

	u32 test[5] = {1,2,3,4,5};
	print<u32[5]>(test);

	Engine eng;
	eng.init();
	eng.loop();
	eng.destroy();

	mem_validate();

	return 0;
}
