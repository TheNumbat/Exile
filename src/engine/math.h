
#pragma once

#include <xmmintrin.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif

#define PI32 3.14159265359f 
#define PI64 3.14159265358979323846 
#define TAU32 (2*PI32) 
#define TAU64 (2*PI64) 

#define RADIANS(v) (v * (PI32 / 180.0f)) 
#define DEGREES(v) (v * (180.0f / PI32)) 

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) < (y) ? (y) : (x))

u32 mod(u32 l, u32 r);
f32 sqrtf(f32 v);
f32 tanf(f32 v);
i32 abs(i32 v);
f32 absf(f32 v);
f64 absf(f64 v);
f32 roundf(f32 v);
f32 ceilf(f32 v);
f32 floorf(f32 v);
i32 floorfi(f32 v);
f32 sinf(f32 v);
f32 cosf(f32 v);
f32 lerpf(f32 min, f32 max, f32 dist);
u32 last_pow_two(u32 val);
u32 next_pow_two(u32 val);

// using templates to specify inner vector types works decently, 
// but not for SIMD, which is only used with floats here.
// hence, it's easier to just make separate types. (even for color, etc.)
// it also works to strongly-typedef everything

union v2 {
	struct {
		f32 x, y;
	};
	f32 a[2] = {};

	void operator+=(v2 v) {x += v.x; y += v.y;}
	void operator-=(v2 v) {x -= v.x; y -= v.y;}
	void operator*=(v2 v) {x *= v.x; y *= v.y;}
	void operator/=(v2 v) {x /= v.x; y /= v.y;}
	void operator*=(f32 s) {x *= s; y *= s;}
	void operator/=(f32 s) {x /= s; y /= s;}
	f32 operator[](u8 idx) {return a[idx];}

	v2() {}
	v2(f32 _x, f32 _y) {x = _x; y = _y;}
	v2(v2& v) {x = v.x; y = v.y;}
};
static_assert(sizeof(v2) == 8, "sizeof(v2) != 8");

union v3 {
	struct {
		f32 x, y, z;
	};
	f32 a[3] = {};

	void operator+=(v3 v) {x += v.x; y += v.y; z += v.z;}
	void operator-=(v3 v) {x -= v.x; y -= v.y; z -= v.z;}
	void operator*=(v3 v) {x *= v.x; y *= v.y; z *= v.z;}
	void operator/=(v3 v) {x /= v.x; y /= v.y; z /= v.z;}
	void operator*=(f32 s) {x *= s; y *= s; z *= s;}
	void operator/=(f32 s) {x /= s; y /= s; z /= s;}
	f32 operator[](u8 idx) {return a[idx];}

	v3() {}
	v3(f32 _x, f32 _y, f32 _z) {x = _x; y = _y; z = _z;}
	v3(v3& v) {x = v.x; y = v.y; z = v.z;}
};
static_assert(sizeof(v3) == 12, "sizeof(v3) != 12");

union uv3 {
	struct {
		u32 x, y, z;
	};
	u32 a[3] = {};

	void operator+=(uv3 v) {x += v.x; y += v.y; z += v.z;}
	void operator-=(uv3 v) {x -= v.x; y -= v.y; z -= v.z;}
	u32 operator[](u8 idx) {return a[idx];}

	uv3() {}
	uv3(u32 _x, u32 _y, u32 _z) {x = _x; y = _y; z = _z;}
	uv3(uv3& v) {x = v.x; y = v.y; z = v.z;}
};
static_assert(sizeof(uv3) == 12, "sizeof(uv3) != 12");

union iv3 {
	struct {
		i32 x, y, z;
	};
	i32 a[3] = {};

	void operator+=(iv3 v) {x += v.x; y += v.y; z += v.z;}
	void operator-=(iv3 v) {x -= v.x; y -= v.y; z -= v.z;}
	i32 operator[](u8 idx) {return a[idx];}

	iv3() {}
	iv3(i32 _x, i32 _y, i32 _z) {x = _x; y = _y; z = _z;}
	iv3(iv3& v) {x = v.x; y = v.y; z = v.z;}
};
static_assert(sizeof(iv3) == 12, "sizeof(iv3) != 12");

union v4 {
	struct {
		f32 x, y, z, w;
	};
	f32 a[4] = {};
	__m128 packed;

	void operator+=(v4 v) {packed = _mm_add_ps(packed, v.packed);}
	void operator-=(v4 v) {packed = _mm_sub_ps(packed, v.packed);}
	void operator*=(v4 v) {packed = _mm_mul_ps(packed, v.packed);}
	void operator/=(v4 v) {packed = _mm_div_ps(packed, v.packed);}
	void operator*=(f32 s) {packed = _mm_mul_ps(packed, _mm_set1_ps(s));}
	void operator/=(f32 s) {packed = _mm_div_ps(packed, _mm_set1_ps(s));}
	f32 operator[](u8 idx) {return a[idx];}

	v4() {}
	v4(f32 _x, f32 _y, f32 _z, f32 _w) {packed = _mm_set_ps(_x, _y, _z, _w);}
	v4(v4& v) {packed = v.packed;}
};
static_assert(sizeof(v4) == 16, "sizeof(v4) != 16");

union bv4 {
	struct {
		u8 x, y, z, w;
	};
	u8 a[4] = {};

	void operator+=(bv4 v) {x += v.x; y += v.y; z += v.z; w += v.w;}
	void operator-=(bv4 v) {x -= v.x; y -= v.y; z -= v.z; w -= v.w;}
	u32 operator[](u8 idx) {return a[idx];}

	bv4() {}
	bv4(u8 _x, u8 _y, u8 _z, u8 _w) {x = _x; y = _y; z = _z; w = _w;}
	bv4(bv4& v) {x = v.x; y = v.y; z = v.z; w = v.w;}
};
static_assert(sizeof(bv4) == 4, "sizeof(bv4) != 4");

union r2 {
	struct {
		f32 x, y, w, h;
	};
	struct {
		v2 xy;
		v2 wh;
	};
	f32 a[4] = {};
	__m128 packed;

	r2() {}
	r2(f32 _x, f32 _y, f32 _w, f32 _h) {packed = _mm_set_ps(_x, _y, _w, _h);}
	r2(r2& r) {packed = r.packed;}
};
static_assert(sizeof(r2) == 16, "sizeof(r2) != 16");

// [Column][Row]
union m4 {
	f32 a[16] = {1, 0, 0, 0,
				 0, 1, 0, 0,
				 0, 0, 1, 0,
				 0, 0, 0, 1}; 
	v4 columns[4];
	__m128 packed[4];

	m4() {}
};
static_assert(sizeof(m4) == 64, "sizeof(m4) != 64");

struct colorf {
	f32 r, g, b, a;

	colorf(f32 _r, f32 _g, f32 _b, f32 _a) {r = _r; g = _g; b = _b; a = _a;}
};
static_assert(sizeof(colorf) == 16, "sizeof(colorf) != 16");

struct color3 {
	u8 r, g, b;

	color3(u8 _r, u8 _g, u8 _b) {r = _r; g = _g; b = _b;}
};
static_assert(sizeof(color3) == 3, "sizeof(color3) != 3");

struct color {
	u8 r, g, b, a;

	colorf to_f() {return colorf(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);}

	color(u8 _r, u8 _g, u8 _b, u8 _a) {r = _r; g = _g; b = _b; a = _a;}
};
static_assert(sizeof(color) == 4, "sizeof(color) != 4");
