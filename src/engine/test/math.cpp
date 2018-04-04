
#pragma warning(disable : 4201)
#pragma warning(disable : 4127)

#include "test.h"

#include "..\util\basic_types.h"
#include "..\math.h"

i32 main() {

	begin();

	ntest("testy test", true);

	// basic sanity tests, none of this is exhaustive or even checks edge cases

	testeq(sqrt(4.0f), 2.0f);
	test(isnan(sqrt(-1.0f)));
	testeq(abs(1.0f), 1.0f);
	testeq(abs(-1.0f), 1.0f);
	testeq(abs(1.0), 1.0);
	testeq(abs(-1.0), 1.0);	
	testeq(round(0.5f), 1.0f);
	testeq(round(0.49f), 0.0f);
	testeq(round(-0.5f), -1.0f);
	testeq(round(-0.49f), 0.0f);
	testeq(ceil(0.1f), 1.0f);
	testeq(ceil(1.0f), 1.0f);
	testeq(ceil(-0.5f), 0.0f);
	testeq(floor(0.1f), 0.0f);
	testeq(floor(1.0f), 1.0f);
	testeq(floor(-0.5f), -1.0f);
	testeq(sin(0.0f), 0.0f);
	testeq(sin(PI32 / 2.0f), 1.0f);
	testeq(sin(PI32), 0.0f);
	testeq(cos(0.0f), 1.0f);
	testeq(cos(PI32 / 2.0f), 0.0f);
	testeq(cos(PI32), -1.0f);
	testeq(tan(0.0f), 0.0f);
	test(isinf(tan(PI32 / 2.0f)));
	testeq(tan(PI32), 0.0f);

	end();
}

