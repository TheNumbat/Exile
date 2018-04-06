
#pragma warning(disable : 4201)
#pragma warning(disable : 4127)

#include "test.h"

#include "..\util\basic_types.h"

#define OSTREAM_OPS
#include "..\math.h"

i32 main() {

	begin();

	// these really don't need to be tested; 
	// they're just using CRT functions currently...
	{
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
		test(abs(sin(PI32)) < 0.0001f);
		testeq(cos(0.0f), 1.0f);
		test(abs(cos(PI32 / 2.0f)) < 0.0001f);
		testeq(cos(PI32), -1.0f);
		testeq(tan(0.0f), 0.0f);
		test(abs(tan(PI32 / 2.0f)) > 1000);
		test(abs(tan(PI32)) < 0.0001f);
	}
	{
		testeq(sqrt({1.0f, 4.0f, 9.0f, 16.0f}), v4(1.0f, 2.0f, 3.0f, 4.0f));
		testeq(abs({-1.0f, 2.0f, -3.0f, 4.0f}), v4(1.0f, 2.0f, 3.0f, 4.0f));
		testeq(round({0.51f, 0.49f, -0.51f, -0.49f}), v4(1.0f, 0.0f, -1.0f, 0.0f));
		testeq(ceil({0.51f, 0.49f, -0.51f, -0.49f}), v4(1.0f, 1.0f, 0.0f, 0.0f));
		testeq(floor({0.51f, 0.49f, -0.51f, -0.49f}), v4(0.0f, 0.0f, -1.0f, -1.0f));
		testfeq(sin({0.0f, PI32 / 2.0f, PI32, 3 * PI32 / 2.0f}), v4(0.0f, 1.0f, 0.0f, -1.0f));
		testfeq(cos({0.0f, PI32 / 2.0f, PI32, 3 * PI32 / 2.0f}), v4(1.0f, 0.0f, -1.0f, 0.0f));
		testeq(lerp({0.0f, 1.0f, 2.0f, 4.0f}, {1.0f, 2.0f, 4.0f, 8.0f}, {0.5f, 0.5f, 0.5f, 0.5f}), v4(0.5f, 1.5f, 3.0f, 6.0f));
		testeq(clamp({0.0f, 1.0f, 2.0f, 4.0f}, {1.0f, 2.0f, 3.0f, 4.0f}, {2.0f, 3.0f, 4.0f, 5.0f}), v4(1.0f, 2.0f, 3.0f, 4.0f));
		testeq(clamp({0.0f, 1.0f, 2.0f, 4.0f}, {-1.0f, -1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 1.0f, 3.0f}), v4(-1.0f, 0.0f, 1.0f, 3.0f));
	}
	{
		testeq(lensq({2.0f, 2.0f, 2.0f, 2.0f}), 16);
		testeq(len({2.0f, 2.0f, 2.0f, 2.0f}), 4);
		testeq(v4(1.0f, 2.0f, 3.0f, 4.0f) + v4(1.0f, 2.0f, 3.0f, 4.0f), 2.0f * v4(1.0f, 2.0f, 3.0f, 4.0f))
		testeq(v4(1.0f, 2.0f, 3.0f, 4.0f) - v4(1.0f, 2.0f, 3.0f, 4.0f), 0.0f * v4(1.0f, 2.0f, 3.0f, 4.0f))
		testeq(v4(1.0f, 2.0f, 3.0f, 4.0f) * v4(1.0f, 2.0f, 3.0f, 4.0f), v4(1.0f, 4.0f, 9.0f, 16.0f))
		testeq(v4(1.0f, 2.0f, 3.0f, 4.0f) / v4(1.0f, 2.0f, 3.0f, 4.0f), v4(1.0f, 1.0f, 1.0f, 1.0f))		
	}
	{
		testeq(m4() * 0.0f, m4::zero);
		testeq(m4() * m4(), m4());
		testeq(m4() * v4(1.0f, 2.0f, 3.0f, 4.0f), v4(1.0f, 2.0f, 3.0f, 4.0f));

		m4 asc = {{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}};
		m4 tasc = {{1,5,9,13},{2,6,10,14},{3,7,11,15},{4,8,12,16}};
		m4 at = {{30,70,110,150},{70,174,278,382},{110,278,446,614},{150,382,614,846}};
		m4 ta = {{276,304,332,360},{304,336,368,400},{332,368,404,440},{360,400,440,480}};

		testeq(transpose(asc), tasc);
		testeq(asc, transpose(tasc));
		testeq(asc * tasc, at);
		testeq(tasc * asc, ta);
	}

	end();
}


