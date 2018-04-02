
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

	v2(f32 _x, f32 _y) {x = _x; y = _y;}
	v2(v2& v) {x = v.x; y = v.y;}
};

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

	v3(f32 _x, f32 _y, f32 _z) {x = _x; y = _y; z = _z;}
	v3(v3& v) {x = v.x; y = v.y; z = v.z;}
};

union uv3 {
	struct {
		u32 x, y, z;
	};
	u32 a[3] = {};

	void operator+=(uv3 v) {x += v.x; y += v.y; z += v.z;}
	void operator-=(uv3 v) {x -= v.x; y -= v.y; z -= v.z;}
	u32 operator[](u8 idx) {return a[idx];}

	uv3(u32 _x, u32 _y, u32 _z) {x = _x; y = _y; z = _z;}
	uv3(uv3& v) {x = v.x; y = v.y; z = v.z;}
};

union v4 {
	struct {
		f32 x, y, z, w;
	};
	f32 a[4] = {};
	__m128 packed;

	void operator+=(v4 v) {x += v.x; y += v.y; z += v.z; w += v.w;}
	void operator-=(v4 v) {x -= v.x; y -= v.y; z -= v.z; w -= v.w;}
	void operator*=(v4 v) {x *= v.x; y *= v.y; z *= v.z; w *= v.w;}
	void operator/=(v4 v) {x /= v.x; y /= v.y; z /= v.z; w /= v.w;}
	void operator*=(f32 s) {x *= s; y *= s; w *= s;}
	void operator/=(f32 s) {x /= s; y /= s; w /= s;}
	f32 operator[](u8 idx) {return a[idx];}
};

// [Column][Row]
union m4 {
	f32 a[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; 
	v4 columns[4];
	__m128 packed[4];
};

struct colorf {
	f32 r, g, b, a;
};

struct color3 {
	u8 r, g, b;
};

struct color {
	u8 r, g, b, a;
};
