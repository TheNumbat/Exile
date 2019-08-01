
#pragma once

#include "basic.h"
#include <math.h>
#include <xmmintrin.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <intrin.h>
#elif defined(__clang__)
#include <smmintrin.h>
#endif

#ifndef IN_MATH_IMPL
#define sqrt _sqrt
#define absv _absv
#define round _round
#define ceil _ceil
#define floor _floor
#define sin _sin
#define cos _cos
#define tan _tan
#define asin _asin
#define acos _acos
#define atan _atan
#define atan2 _atan2
#define pow _pow
#endif

#define PI32 3.14159265358979323846264338327950288f
#define PI64 3.14159265358979323846264338327950288
#define TAU32 (2.0f*PI32) 
#define TAU64 (2.0*PI64) 

#define RADIANS(v) (v * (PI32 / 180.0f)) 
#define DEGREES(v) (v * (180.0f / PI32)) 

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) < (y) ? (y) : (x))
#define min3(x,y,z) (min(min(x,y), z))
#define max3(x,y,z) (max(max(x,y), z))

#define EPSILON32 0.00001f
#define EPSILON64 0.00001

union m4; 
union v4; union bv4; 
union v2; union uv2; union iv2;
union v3; union iv3; union uv3; 
union r2; union ur2; union ir2;
struct color; struct color3; struct colorf;

u32 mod(u32 l, u32 r);
u64 mod(u64 l, u64 r);
f32 mod(f32 l, f32 r);
f64 mod(f64 l, f64 r);
f32 _pow(f32 l, f32 r);
f64 _pow(f64 l, f64 r);
f32 fract(f32 v);
f64 fract(f64 v);
f32 _sqrt(f32 v);
f32 _absv(f32 v);
f64 _absv(f64 v);
f32 _round(f32 v);
f32 _ceil(f32 v);
f32 _floor(f32 v);
f32 _sin(f32 v);
f32 _cos(f32 v);
f32 _tan(f32 v);
f32 _asin(f32 v);
f32 _acos(f32 v);
f32 _atan(f32 v);
f32 _atan2(f32 l, f32 r);
f32 lerp(f32 min, f32 max, f32 dist);
f32 clamp(f32 val, f32 min, f32 max);
u32 last_pow_two(u32 val);
u32 next_pow_two(u32 val);

f32 smoothstep(f32 e0, f32 e1, f32 x);
f32 perlin(f32 x, f32 y, f32 z, i32 x_wrap, i32 y_wrap, i32 z_wrap);
f32 perlin_grad(i32 hash, f32 x, f32 y, f32 z);

v3 _absv(v3 v);
v4 _sqrt(v4 v);
v4 _absv(v4 v);
v4 _round(v4 v);
v4 _ceil(v4 v);
v4 _floor(v4 v);
v4 _sin(v4 v); // NOTE(max): SSE sin/cos from sse_mathfun
v4 _cos(v4 v);
v4 step(v4 l, v4 r);
v4 fract(v4 v);
v4 lerp(v4 min, v4 max, v4 dist);
v4 clamp(v4 val, v4 min, v4 max);

f32 lensq(v2 v);
f32 lensq(v3 v);
f32 lensq(v4 v);
f32 len(v2 v);
f32 len(v3 v);
f32 len(v4 v);
f32 min_reset(v3 v);
v2 minv2(v2 l, v2 r);
v2 maxv2(v2 l, v2 r);
bool all_less(v3 v, f32 c);

v2 norm(v2 v);
v3 norm(v3 v);
v4 norm(v4 v);

f32 dot(v2 l, v2 r);
f32 dot(v3 l, v3 r);
f32 dot(v4 l, v4 r);
v3 cross(v3 l, v3 r);

v2 operator+(v2 l, v2 r);
v3 operator+(v3 l, v3 r);
v4 operator+(v4 l, v4 r);
uv2 operator+(uv2 l, uv2 r);
iv2 operator+(iv2 l, iv2 r);
uv3 operator+(uv3 l, uv3 r);
iv3 operator+(iv3 l, iv3 r);
bv4 operator+(bv4 l, bv4 r);
v2 operator-(v2 l, v2 r);
v3 operator-(v3 l, v3 r);
v4 operator-(v4 l, v4 r);
uv2 operator-(uv2 l, uv2 r);
iv2 operator-(iv2 l, iv2 r);
uv3 operator-(uv3 l, uv3 r);
iv3 operator-(iv3 l, iv3 r);
bv4 operator-(bv4 l, bv4 r);
v2 operator*(v2 l, v2 r);
v3 operator*(v3 l, v3 r);
v4 operator*(v4 l, v4 r);
uv2 operator*(uv2 l, uv2 r);
iv2 operator*(iv2 l, iv2 r);
uv3 operator*(uv3 l, uv3 r);
iv3 operator*(iv3 l, iv3 r);
bv4 operator*(bv4 l, bv4 r);
v2 operator*(v2 l, f32 r);
v3 operator*(v3 l, f32 r);
v3 operator*(v3 l, i32 r);
v4 operator*(v4 l, f32 r);
uv2 operator*(uv2 l, u32 r);
iv2 operator*(iv2 l, i32 r);
uv3 operator*(uv3 l, u32 r);
iv3 operator*(iv3 l, i32 r);
v3 operator*(iv3 l, f32 r);
bv4 operator*(bv4 l, u8 r);
v2 operator*(f32 l, v2 r);
v3 operator*(f32 l, v3 r);
v4 operator*(f32 l, v4 r);
uv2 operator*(u32 l, uv2 r);
iv2 operator*(i32 l, iv2 r);
uv3 operator*(u32 l, uv3 r);
iv3 operator*(i32 l, iv3 r);
bv4 operator*(u8 l, bv4 r);
v2 operator/(v2 l, v2 r);
v3 operator/(v3 l, v3 r);
v4 operator/(v4 l, v4 r);
uv2 operator/(uv2 l, uv2 r);
iv2 operator/(iv2 l, iv2 r);
uv3 operator/(uv3 l, uv3 r);
iv3 operator/(iv3 l, iv3 r);
bv4 operator/(bv4 l, bv4 r);
v2 operator/(v2 l, f32 r);
v3 operator/(v3 l, f32 r);
v4 operator/(v4 l, f32 r);
uv2 operator/(uv2 l, u32 r);
iv2 operator/(iv2 l, i32 r);
uv3 operator/(uv3 l, u32 r);
iv3 operator/(iv3 l, i32 r);
bv4 operator/(bv4 l, u8 r);
v2 operator/(f32 l, v2 r);
v3 operator/(f32 l, v3 r);
v4 operator/(f32 l, v4 r);
uv2 operator/(u32 l, uv2 r);
iv2 operator/(i32 l, iv2 r);
uv3 operator/(u32 l, uv3 r);
iv3 operator/(i32 l, iv3 r);
bv4 operator/(u8 l, bv4 r);

r2 operator+(r2 l, r2 r);
r2 operator+(r2 l, v2 r);
r2 operator-(r2 l, r2 r);
r2 operator-(r2 l, v2 r);
r2 operator*(r2 l, f32 r);
ur2 operator+(ur2 l, ur2 r);
ur2 operator+(ur2 l, uv2 r);
ur2 operator-(ur2 l, ur2 r);
ur2 operator-(ur2 l, uv2 r);
ur2 operator*(ur2 l, u32 r);
ir2 operator+(ir2 l, ir2 r);
ir2 operator+(ir2 l, iv2 r);
ir2 operator-(ir2 l, ir2 r);
ir2 operator-(ir2 l, iv2 r);
ir2 operator*(ir2 l, i32 r);

bool inside(r2 l, v2 r);
bool intersect(r2 l, r2 r);

bool operator==(v2 l, v2 r);
bool operator==(uv2 l, uv2 r);
bool operator==(iv2 l, iv2 r);
bool operator==(v3 l, v3 r);
bool operator==(iv3 l, iv3 r);
bool operator==(uv3 l, uv3 r);
bool operator==(v4 l, v4 r);
bool operator==(bv4 l, bv4 r);
bool operator==(m4 l, m4 r);
bool operator==(r2 l, r2 r);
bool operator==(ur2 l, ur2 r);
bool operator==(ir2 l, ir2 r);
bool operator==(color l, color r);
bool operator==(color3 l, color3 r);
bool operator==(colorf l, colorf r);
bool operator!=(v2 l, v2 r);
bool operator!=(uv2 l, uv2 r);
bool operator!=(iv2 l, iv2 r);
bool operator!=(v3 l, v3 r);
bool operator!=(iv3 l, iv3 r);
bool operator!=(uv3 l, uv3 r);
bool operator!=(v4 l, v4 r);
bool operator!=(bv4 l, bv4 r);
bool operator!=(m4 l, m4 r);
bool operator!=(r2 l, r2 r);
bool operator!=(ur2 l, ur2 r);
bool operator!=(ir2 l, ir2 r);
bool operator!=(color l, color r);
bool operator!=(color3 l, color3 r);
bool operator!=(colorf l, colorf r);
bool fudge(f32 l, f32 r);
bool fudge(v2 l, v2 r);
bool fudge(v3 l, v3 r);
bool fudge(v4 l, v4 r);

#ifdef OSTREAM_OPS
#include <ostream>
std::ostream& operator<<(std::ostream& out, v2 r);
std::ostream& operator<<(std::ostream& out, uv2 r);
std::ostream& operator<<(std::ostream& out, iv2 r);
std::ostream& operator<<(std::ostream& out, v3 r);
std::ostream& operator<<(std::ostream& out, iv3 r);
std::ostream& operator<<(std::ostream& out, uv3 r);
std::ostream& operator<<(std::ostream& out, v4 r);
std::ostream& operator<<(std::ostream& out, bv4 r);
std::ostream& operator<<(std::ostream& out, m4 r);
std::ostream& operator<<(std::ostream& out, r2 r);
std::ostream& operator<<(std::ostream& out, ur2 r);
std::ostream& operator<<(std::ostream& out, ir2 r);
std::ostream& operator<<(std::ostream& out, color r);
std::ostream& operator<<(std::ostream& out, color3 r);
std::ostream& operator<<(std::ostream& out, colorf r);
#endif

m4 operator+(m4 l, m4 r);
m4 operator-(m4 l, m4 r);
m4 operator*(m4 l, m4 r);
v4 operator*(m4 l, v4 r);
m4 operator*(m4 l, f32 r);
m4 operator/(m4 l, f32 r);
m4 transpose(m4 m);
m4 inverse(m4 m);
m4 ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
m4 project(f32 fov, f32 ar, f32 n);
m4 translate(v3 v);
m4 rotate(f32 a, v3 axis);
m4 scale(v3 s);
m4 lookAt(v3 pos, v3 at, v3 up);

void rand_init(u32 seed);
i32  randi();
u32  randu();	
f32  randf();
f64  randf64();
v3 	 rand_unit();

// using templates to specify inner vector types works decently, 
// but not for SIMD, which is only used with floats here.
// hence, it's easier to just make separate types. (even for color, etc.)
// it also works to strongly-typedef everything

union iv2 {
	struct {
		i32 x, y;
	};
	i32 a[2] = {};

	void operator+=(iv2 v) {x += v.x; y += v.y;}
	void operator-=(iv2 v) {x -= v.x; y -= v.y;}
	void operator*=(iv2 v) {x *= v.x; y *= v.y;}
	void operator/=(iv2 v) {x /= v.x; y /= v.y;}
	void operator*=(i32 s) {x *= s; y *= s;}
	void operator/=(i32 s) {x /= s; y /= s;}
	i32& operator[](i32 idx) {return a[idx];}

	iv2() {}
	iv2(i32 _x) {x = _x; y = _x;}
	iv2(i32 _x, i32 _y) {x = _x; y = _y;}
	iv2(iv2& v) {*this = v;}
	iv2(iv2&& v) {*this = v;}
	iv2& operator=(iv2& v) {x = v.x; y = v.y; return *this;}
	iv2& operator=(iv2&& v) {x = v.x; y = v.y; return *this;}
};
static_assert(sizeof(iv2) == 8, "sizeof(iv2) != 8");

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
	f32& operator[](i32 idx) {return a[idx];}
	iv2 to_i() {return iv2((i32)_round(x), (i32)_round(y));}

	v2() {}
	v2(f32 _x) {x = _x; y = _x;}
	v2(f32 _x, f32 _y) {x = _x; y = _y;}
	v2(i32 _x, i32 _y) {x = (f32)_x; y = (f32)_y;}
	v2(v2& v) {*this = v;}
	v2(v2&& v) {*this = v;}
	v2& operator=(v2& v) {x = v.x; y = v.y; return *this;}
	v2& operator=(v2&& v) {x = v.x; y = v.y; return *this;}
};
static_assert(sizeof(v2) == 8, "sizeof(v2) != 8");

union uv2 {
	struct {
		u32 x, y;
	};
	u32 a[2] = {};

	void operator+=(uv2 v) {x += v.x; y += v.y;}
	void operator-=(uv2 v) {x -= v.x; y -= v.y;}
	void operator*=(uv2 v) {x *= v.x; y *= v.y;}
	void operator/=(uv2 v) {x /= v.x; y /= v.y;}
	void operator*=(u32 s) {x *= s; y *= s;}
	void operator/=(u32 s) {x /= s; y /= s;}
	u32& operator[](i32 idx) {return a[idx];}

	uv2() {}
	uv2(u32 _x, u32 _y) {x = _x; y = _y;}
	uv2(uv2& v) {*this = v;}
	uv2(uv2&& v) {*this = v;}
	uv2& operator=(uv2& v) {x = v.x; y = v.y; return *this;}
	uv2& operator=(uv2&& v) {x = v.x; y = v.y; return *this;}
};
static_assert(sizeof(uv2) == 8, "sizeof(uv2) != 8");

union v3;
union iv3 {
	struct {
		i32 x, y, z;
	};
	i32 a[3] = {};

	void operator+=(iv3 v) {x += v.x; y += v.y; z += v.z;}
	void operator-=(iv3 v) {x -= v.x; y -= v.y; z -= v.z;}
	void operator*=(i32 s) {x *= s; y *= s; z *= s;}
	i32& operator[](i32 idx) {return a[idx];}
	v3 to_f();

	iv3() {}
	iv3(i32 _x, i32 _y, i32 _z) {x = _x; y = _y; z = _z;}
	iv3(iv3& v) {*this = v;}
	iv3(iv3&& v) {*this = v;}
	iv3& operator=(iv3& v) {x = v.x; y = v.y; z = v.z; return *this;}
	iv3& operator=(iv3&& v) {x = v.x; y = v.y; z = v.z; return *this;}
};
static_assert(sizeof(iv3) == 12, "sizeof(iv3) != 12");

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
	f32& operator[](i32 idx) {return a[idx];}
	v3 operator-() {return {-x,-y,-z};}
	operator bool() {return x || y || z;}
	iv3 to_i() {return iv3((i32)_round(x), (i32)_round(y), (i32)_round(z));}
	iv3 to_i_t() {return iv3((i32)x, (i32)y, (i32)z) - iv3(x < 0 ? 1 : 0, y < 0 ? 1 : 0, z < 0 ? 1 : 0);}

	v3() {}
	v3(f32 _x) {x = _x; y = _x; z = _x;}
	v3(f32 _x, f32 _y, f32 _z) {x = _x; y = _y; z = _z;}
	v3(i32 _x, i32 _y, i32 _z) {x = (f32)_x; y = (f32)_y; z = (f32)_z;}
	v3(v3& v) {*this = v;}
	v3(v3&& v) {*this = v;}
	v3& operator=(v3& v) {x = v.x; y = v.y; z = v.z; return *this;}
	v3& operator=(v3&& v) {x = v.x; y = v.y; z = v.z; return *this;}
};
static_assert(sizeof(v3) == 12, "sizeof(v3) != 12");

union uv3 {
	struct {
		u32 x, y, z;
	};
	u32 a[3] = {};

	void operator+=(uv3 v) {x += v.x; y += v.y; z += v.z;}
	void operator-=(uv3 v) {x -= v.x; y -= v.y; z -= v.z;}
	u32& operator[](i32 idx) {return a[idx];}

	uv3() {}
	uv3(u32 _x, u32 _y, u32 _z) {x = _x; y = _y; z = _z;}
	uv3(uv3& v) {*this = v;}
	uv3(uv3&& v) {*this = v;}
	uv3& operator=(uv3& v) {x = v.x; y = v.y; z = v.z; return *this;}
	uv3& operator=(uv3&& v) {x = v.x; y = v.y; z = v.z; return *this;}
};
static_assert(sizeof(uv3) == 12, "sizeof(uv3) != 12");

union v4 {
	struct {
		f32 x, y, z, w;
	};
	struct {
		v2 xy, zw;
	};
	struct {
		v3 xyz;
		f32 _w;
	};
	f32 a[4] = {};
	__m128 packed;

	void operator+=(v4 v) {packed = _mm_add_ps(packed, v.packed);}
	void operator-=(v4 v) {packed = _mm_sub_ps(packed, v.packed);}
	void operator*=(v4 v) {packed = _mm_mul_ps(packed, v.packed);}
	void operator/=(v4 v) {packed = _mm_div_ps(packed, v.packed);}
	void operator*=(f32 s) {packed = _mm_mul_ps(packed, _mm_set1_ps(s));}
	void operator/=(f32 s) {packed = _mm_div_ps(packed, _mm_set1_ps(s));}
	f32& operator[](i32 idx) {return a[idx];}
	operator iv3() {return {(i32)x, (i32)y, (i32)z};}

	v4() {}
	v4(f32 _v) {packed = _mm_set_ps(_v, _v, _v, _v);}
	v4(v3 _v, f32 _w) {packed = _mm_set_ps(_w, _v.z, _v.y, _v.x);}
	v4(f32 _x, f32 _y, f32 _z, f32 _w) {packed = _mm_set_ps(_w, _z, _y, _x);}
	v4(__m128 p) {packed = p;}
	v4(v4& v) {*this = v;}
	v4(v4&& v) {*this = v;}
	v4& operator=(v4& v) {packed = v.packed; return *this;}
	v4& operator=(v4&& v) {packed = v.packed; return *this;}
};
static_assert(sizeof(v4) == 16, "sizeof(v4) != 16");
static_assert(alignof(v4) == 16, "alignof(v4) != 16");

union bv4 {
	struct {
		u8 x, y, z, w;
	};
	u8 a[4] = {};

	void operator+=(bv4 v) {x += v.x; y += v.y; z += v.z; w += v.w;}
	void operator-=(bv4 v) {x -= v.x; y -= v.y; z -= v.z; w -= v.w;}
	u8& operator[](i32 idx) {return a[idx];}
	bool all_same() {return x == y && x == z && x == w;}

	bv4() {}
	bv4(u8 _x, u8 _y, u8 _z, u8 _w) {x = _x; y = _y; z = _z; w = _w;}
	bv4(bv4& v) {*this = v;}
	bv4(bv4&& v) {*this = v;}
	bv4& operator=(bv4& v) {x = v.x; y = v.y; z = v.z; w = v.w; return *this;}
	bv4& operator=(bv4&& v) {x = v.x; y = v.y; z = v.z; w = v.w; return *this;}
};
static_assert(sizeof(bv4) == 4, "sizeof(bv4) != 4");

union ir2 {
	struct {
		i32 x, y, w, h;
	};
	struct {
		iv2 xy;
		iv2 wh;
	};
	i32 a[4] = {};

	ir2() {}
	ir2(i32 _x, i32 _y, i32 _w, i32 _h) {x = _x; y = _y; w = _w; h = _h;}
	ir2(iv2 _xy, iv2 _wh) {xy = _xy; wh = _wh;}
	ir2(ir2& r) {*this = r;}
	ir2(ir2&& r) {*this = r;}
	ir2& operator=(ir2& r) {x = r.x; y = r.y; w = r.w; h = r.h; return *this;}
	ir2& operator=(ir2&& r) {x = r.x; y = r.y; w = r.w; h = r.h; return *this;}
};
static_assert(sizeof(ir2) == 16, "sizeof(ir2) != 16");

union ur2 {
	struct {
		u32 x, y, w, h;
	};
	struct {
		uv2 xy;
		uv2 wh;
	};
	u32 a[4] = {};

	ur2() {}
	ur2(u32 _x, u32 _y, u32 _w, u32 _h) {x = _x; y = _y; w = _w; h = _h;}
	ur2(uv2 _xy, uv2 _wh) {xy = _xy; wh = _wh;}
	ur2(v4 v) {x = (u32)v.x; y = (u32)v.y; w = (u32)v.z; h = (u32)v.w;}
	ur2(ur2& r) {*this = r;}
	ur2(ur2&& r) {*this = r;}
	ur2& operator=(ur2& r) {x = r.x; y = r.y; w = r.w; h = r.h; return *this;}
	ur2& operator=(ur2&& r) {x = r.x; y = r.y; w = r.w; h = r.h; return *this;}
};
static_assert(sizeof(ur2) == 16, "sizeof(ur2) != 16");

union r2 {
	struct {
		f32 x, y, w, h;
	};
	struct {
		v2 xy;
		v2 wh;
	};
	v4 vec;
	f32 a[4] = {};
	__m128 packed;

	ur2 to_u() {return ur2(_round(vec));}

	r2() {}
	r2(f32 _x, f32 _y, f32 _w, f32 _h) {packed = _mm_set_ps(_h, _w, _y, _x);}
	r2(v2 _xy, v2 _wh) {xy = _xy; wh = _wh;}
	r2(__m128 p) {packed = p;}
	r2(r2& r) {*this = r;}
	r2(r2&& r) {*this = r;}
	r2& operator=(r2& r) {packed = r.packed; return *this;}
	r2& operator=(r2&& r) {packed = r.packed; return *this;}
};
static_assert(sizeof(r2) == 16, "sizeof(r2) != 16");
static_assert(alignof(r2) == 16, "alignof(r2) != 16");

// [Column][Row]
union m4 {
	f32 a[16] = {1, 0, 0, 0,
				 0, 1, 0, 0,
				 0, 0, 1, 0,
				 0, 0, 0, 1}; 
	v4 columns[4];
	__m128 packed[4];

	void operator+=(m4 v) {for(i32 i = 0; i < 4; i++) packed[i] = _mm_add_ps(packed[i], v.packed[i]);}
	void operator-=(m4 v) {for(i32 i = 0; i < 4; i++) packed[i] = _mm_sub_ps(packed[i], v.packed[i]);}
	void operator*=(m4 v) {*this = *this * v;}
	void operator*=(f32 s) {__m128 mul = _mm_set1_ps(s); for(i32 i = 0; i < 4; i++) packed[i] = _mm_mul_ps(packed[i], mul);}
	void operator/=(f32 s) {__m128 div = _mm_set1_ps(s); for(i32 i = 0; i < 4; i++) packed[i] = _mm_div_ps(packed[i], div);}
	v4& operator[](i32 idx) {return columns[idx];}

	m4() {}
	m4(v4 c0, v4 c1, v4 c2, v4 c3) {columns[0] = c0; columns[1] = c1; columns[2] = c2; columns[3] = c3;}
	m4(m4& m) {*this = m;}
	m4(m4&& m) {*this = m;}
	m4& operator=(m4& m) {for(i32 i = 0; i < 4; i++) packed[i] = m.packed[i]; return *this;}
	m4& operator=(m4&& m) {for(i32 i = 0; i < 4; i++) packed[i] = m.packed[i]; return *this;}

	static m4 zero;
	static m4 I;
};
static_assert(sizeof(m4) == 64, "sizeof(m4) != 64");
static_assert(alignof(m4) == 16, "alignof(m4) != 16");

struct colorf {
	f32 r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

	colorf() {}
	colorf(f32 _r, f32 _g, f32 _b, f32 _a) {r = _r; g = _g; b = _b; a = _a;}
};
static_assert(sizeof(colorf) == 16, "sizeof(colorf) != 16");

struct color3 {
	u8 r = 0, g = 0, b = 0;

	color3() {}
	color3(u8 _r, u8 _g, u8 _b) {r = _r; g = _g; b = _b;}
};
static_assert(sizeof(color3) == 3, "sizeof(color3) != 3");

struct color {
	u8 r = 0, g = 0, b = 0, a = 0;

	colorf to_f() {return colorf(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);}

	color() {}
	color(u8 _r, u8 _g, u8 _b, u8 _a) {r = _r; g = _g; b = _b; a = _a;}
	color(color3 c3, u8 _a) {r = c3.r; g = c3.g; b = c3.b; a = _a;}
};
static_assert(sizeof(color) == 4, "sizeof(color) != 4");

const color WHITE = color(255, 255, 255, 255);
const color BLACK = color(0, 0, 0, 255);
const color RED   = color(255, 0, 0, 255);
const color GREEN = color(0, 255, 0, 255);
const color BLUE  = color(0, 0, 255, 255);


