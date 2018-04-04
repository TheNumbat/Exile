
#pragma once

#include <xmmintrin.h>

#define PI32 3.14159265359f 
#define PI64 3.14159265358979323846 
#define TAU32 (2.0f*PI32) 
#define TAU64 (2.0*PI64) 

#define RADIANS(v) (v * (PI32 / 180.0f)) 
#define DEGREES(v) (v * (180.0f / PI32)) 

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) < (y) ? (y) : (x))

union v2; union uv2; union v3; union iv3; union uv3; union v4; union bv4; union m4; union r2; union ur2;

u32 mod(u32 l, u32 r);
f32 sqrtf(f32 v);
f32 tanf(f32 v);
i32 abs(i32 v);
f32 absf(f32 v);
f64 absf(f64 v);
f32 roundf(f32 v);
f32 ceilf(f32 v);
f32 floorf(f32 v);
f32 sinf(f32 v);
f32 cosf(f32 v);
f32 tanf(f32 v);
f32 asinf(f32 v);
f32 acosf(f32 v);
f32 atanf(f32 v);
f32 lerpf(f32 min, f32 max, f32 dist);
f32 clampf(f32 val, f32 min, f32 max);
u32 last_pow_two(u32 val);
u32 next_pow_two(u32 val);

f32 perlin(f32 x, f32 y, f32 z, i32 x_wrap, i32 y_wrap, i32 z_wrap);
f32 perlin_grad(i32 hash, f32 x, f32 y, f32 z);

v4 sqrtf(v4 v);
v4 tanf(v4 v);
v4 absf(v4 v);
v4 roundf(v4 v);
v4 ceilf(v4 v);
v4 floorf(v4 v);
v4 floorf(v4 v);
v4 floorf(v4 v);
v4 sinf(v4 v);
v4 cosf(v4 v);
v4 tanf(v4 v);
v4 asinf(v4 v);
v4 acosf(v4 v);
v4 atanf(v4 v);
v4 lerpf(v4 min, v4 max, v4 dist);
v4 clampf(v4 val, v4 min, v4 max);

f32 lensq(v2 v);
f32 lensq(v3 v);
f32 lensq(v4 v);
f32 len(v2 v);
f32 len(v3 v);
f32 len(v4 v);

v2 norm(v2 v);
v3 norm(v3 v);
v4 norm(v4 v);

f32 dot(v2 l, v2 r);
f32 dot(v3 l, v3 r);
f32 dot(v4 l, v4 r);
v3 cross(v3 l, v3 r);

v2 operator+(v2 l, v2 r);
v4 operator+(v4 l, v4 r);
v3 operator+(v3 l, v3 r);
uv2 operator+(uv2 l, uv2 r);
uv3 operator+(uv3 l, uv3 r);
iv3 operator+(iv3 l, iv3 r);
bv4 operator+(bv4 l, bv4 r);
v2 operator-(v2 l, v2 r);
v3 operator-(v3 l, v3 r);
v4 operator-(v4 l, v4 r);
uv2 operator-(uv2 l, uv2 r);
uv3 operator-(uv3 l, uv3 r);
iv3 operator-(iv3 l, iv3 r);
bv4 operator-(bv4 l, bv4 r);
v2 operator*(v2 l, v2 r);
v3 operator*(v3 l, v3 r);
v4 operator*(v4 l, v4 r);
uv2 operator*(uv2 l, uv2 r);
uv3 operator*(uv3 l, uv3 r);
iv3 operator*(iv3 l, iv3 r);
bv4 operator*(bv4 l, bv4 r);
v2 operator*(v2 l, f32 r);
v3 operator*(v3 l, f32 r);
v4 operator*(v4 l, f32 r);
uv2 operator*(uv2 l, u32 r);
uv3 operator*(uv3 l, u32 r);
iv3 operator*(iv3 l, i32 r);
bv4 operator*(bv4 l, u8 r);
v2 operator/(v2 l, v2 r);
v3 operator/(v3 l, v3 r);
v4 operator/(v4 l, v4 r);
uv2 operator/(uv2 l, uv2 r);
uv3 operator/(uv3 l, uv3 r);
iv3 operator/(iv3 l, iv3 r);
bv4 operator/(bv4 l, bv4 r);
v2 operator/(v2 l, f32 r);
v3 operator/(v3 l, f32 r);
v4 operator/(v4 l, f32 r);
uv2 operator/(uv2 l, u32 r);
uv3 operator/(uv3 l, u32 r);
iv3 operator/(iv3 l, i32 r);
bv4 operator/(bv4 l, u8 r);

r2 operator+(r2 l, r2 r);
r2 operator+(r2 l, v2 r);
r2 operator-(r2 l, r2 r);
r2 operator-(r2 l, v2 r);
r2 operator*(r2 l, f32 r);
ur2 operator+(ur2 l, ur2 r);
ur2 operator+(ur2 l, uv2 r);
ur2 operator-(ur2 l, ur2 r);
ur2 operator-(ur2 l, uv2 r);
ur2 operator*(ur2 l, f32 r);

bool inside(r2 l, v2 r);
bool intersect(r2 l, r2 r);

m4 operator+(m4 l, m4 r);
m4 operator-(m4 l, m4 r);
m4 operator*(m4 l, m4 r);
m4 operator*(m4 l, v4 r);
m4 operator*(m4 l, f32 r);
m4 operator/(m4 l, f32 r);
m4 transpose(m4 m);
m4 ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
m4 proj(f32 fov, f32 ar, f32 n, f32 f);
m4 translate(v3 v);
m4 rotate(f32 a, v3 axis);
m4 scale(v3 s);
m4 lookAt(v3 pos, v3 at, v3 up);

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
	u32 operator[](u8 idx) {return a[idx];}

	uv2() {}
	uv2(u32 _x, u32 _y) {x = _x; y = _y;}
	uv2(uv2& v) {*this = v;}
	uv2(uv2&& v) {*this = v;}
	uv2& operator=(uv2& v) {x = v.x; y = v.y; return *this;}
	uv2& operator=(uv2&& v) {x = v.x; y = v.y; return *this;}
};
static_assert(sizeof(uv2) == 8, "sizeof(uv2) != 8");

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
	u32 operator[](u8 idx) {return a[idx];}

	uv3() {}
	uv3(u32 _x, u32 _y, u32 _z) {x = _x; y = _y; z = _z;}
	uv3(uv3& v) {*this = v;}
	uv3(uv3&& v) {*this = v;}
	uv3& operator=(uv3& v) {x = v.x; y = v.y; z = v.z; return *this;}
	uv3& operator=(uv3&& v) {x = v.x; y = v.y; z = v.z; return *this;}
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
	iv3(iv3& v) {*this = v;}
	iv3(iv3&& v) {*this = v;}
	iv3& operator=(iv3& v) {x = v.x; y = v.y; z = v.z; return *this;}
	iv3& operator=(iv3&& v) {x = v.x; y = v.y; z = v.z; return *this;}
};
static_assert(sizeof(iv3) == 12, "sizeof(iv3) != 12");

union v4 {
	struct {
		f32 x, y, z, w;
	};
	struct {
		v2 xy, zw;
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
	v4(v4& v) {*this = v;}
	v4(v4&& v) {*this = v;}
	v4& operator=(v4& v) {packed = v.packed; return *this;}
	v4& operator=(v4&& v) {packed = v.packed; return *this;}
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
	bv4(bv4& v) {*this = v;}
	bv4(bv4&& v) {*this = v;}
	bv4& operator=(bv4& v) {x = v.x; y = v.y; z = v.z; w = v.w; return *this;}
	bv4& operator=(bv4&& v) {x = v.x; y = v.y; z = v.z; w = v.w; return *this;}
};
static_assert(sizeof(bv4) == 4, "sizeof(bv4) != 4");

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

	ur2 to_u() {return ur2(roundf(vec));}

	r2() {}
	r2(f32 _x, f32 _y, f32 _w, f32 _h) {packed = _mm_set_ps(_x, _y, _w, _h);}
	r2(v2 _xy, v2 _wh) {xy = _xy; wh = _wh;}
	r2(r2& r) {*this = r;}
	r2(r2&& r) {*this = r;}
	r2& operator=(r2& r) {packed = r.packed; return *this;}
	r2& operator=(r2&& r) {packed = r.packed; return *this;}
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

	void operator+=(m4 v) {for(u8 i = 0; i < 4; i++) packed[i] = _mm_add_ps(packed[i], v.packed[i]);}
	void operator-=(m4 v) {for(u8 i = 0; i < 4; i++) packed[i] = _mm_sub_ps(packed[i], v.packed[i]);}
	void operator*=(m4 v) {*this = *this * v;}
	void operator*=(f32 s) {__m128 mul = _mm_set1_ps(s); for(u8 i = 0; i < 4; i++) packed[i] = _mm_mul_ps(packed[i], mul);}
	void operator/=(f32 s) {__m128 div = _mm_set1_ps(s); for(u8 i = 0; i < 4; i++) packed[i] = _mm_div_ps(packed[i], div);}
	v4 operator[](u8 idx) {return columns[idx];}

	m4() {}
	m4(m4& m) {*this = m;}
	m4(m4&& m) {*this = m;}
	m4& operator=(m4& m) {for(u8 i = 0; i < 4; i++) packed[i] = m.packed[i]; return *this;}
	m4& operator=(m4&& m) {for(u8 i = 0; i < 4; i++) packed[i] = m.packed[i]; return *this;}
};
static_assert(sizeof(m4) == 64, "sizeof(m4) != 64");

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
