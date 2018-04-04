
#pragma warning(disable : 4201)
#pragma warning(disable : 4127)

#include "test.h"

#include "..\util\basic_types.h"
#include "..\math.h"

i32 main() {

	begin();

	ntest("testy test", true);

	test(1 == 1);
	test(1 == 1, 1 == 1);
	test(1 != 1);

	end();
}
