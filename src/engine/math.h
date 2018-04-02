
#pragma once

#include <math.h>

// TODO(max): remove cmath, use more SIMD intrinsics
	// apply to v2_t, v3_t, v4_t, m4_t ops
		// use for packed normal math ops
	// make v2_t_4x, v3_t_4x, v4_t_4x, m4_t_4x
	// mod, div, sign, trig, bit-scanning
// TODO(max): quaternions
// TODO(max): get rid of template for matrix

#define PI32 3.14159265359f 
#define PI64 3.14159265358979323846 
#define TAU32 (2*PI32) 
#define TAU64 (2*PI64) 
#define RADIANS(v) (v * (PI32 / 180.0f)) 
#define DEGREES(v) (v * (180.0f / PI32)) 
#define sqrtf(v) _sqrtf(v) 
#define tanf(v) _tanf(v) 
#define abs(v) _abs(v) 
#define absf(v) absf_(v) 
#define roundf(v) _roundf(v) 
#define ceilf(v) _ceilf(v) 
#define floorf(v) _floorf(v) 
#define sinf(v) _sinf(v) 
#define cosf(v) _cosf(v) 
#define lerpf(i,m,d) _lerpf(i,m,d) 
#define min(x,y) (x < y ? x : y)
#define max(x,y) (x < y ? y : x)

template<typename T>
union v2_t {
	struct {
		T x;
		T y;
	};
	struct {
		T u;
		T v;
	};
	T f[2] = {};
	v2_t() {};
};
typedef v2_t<f32> v2;
typedef v2_t<i32> iv2;
typedef v2_t<u32> uv2;
typedef v2_t<u8>  bv2;
typedef v2_t<f64> dv2;
template union v2_t<f32>;
template union v2_t<i32>;
template union v2_t<u32>;
template union v2_t<u8>;
template union v2_t<f64>;

template<typename T>
union r2_t {
	struct {
		T x;
		T y;
		T w;
		T h;	
	};
	struct {
		v2_t<T> xy;
		v2_t<T> wh;
	};
	T v[4] = {};
	r2_t() {};
};
typedef r2_t<f32> r2;
typedef r2_t<i32> ir2;
typedef r2_t<u32> ur2;
typedef	r2_t<u8>  br2;
typedef r2_t<u16> sr2;
template union r2_t<f32>;
template union r2_t<i32>;
template union r2_t<u32>;
template union r2_t<u8>;

template<typename T>
union v3_t {
	struct {
		T x;
		T y;
		T z;
	};
	struct {
		T r;
		T g;
		T b;
	};
	struct {
		v2_t<T> xy;
		T __ignore1;
	};
	struct {
		T __ignore2;
		v2_t<T> yz;
	};
	T f[3] = {};
	v3_t() {};

	inline void operator+=(v3_t<T> v);
	inline void operator-=(v3_t<T> v);
	inline void operator*=(v3_t<T> v);
	inline void operator*=(T s);
	inline void operator/=(v3_t<T> v);
	inline void operator/=(T s);
};
typedef v3_t<f32> v3;
typedef v3_t<i32> iv3;
typedef v3_t<u32> uv3;
typedef v3_t<u8>  bv3;
typedef bv3 	  color3;
typedef v3 		  color3f;
template union v3_t<f32>;
template union v3_t<i32>;
template union v3_t<u32>;
template union v3_t<u8>;

template<typename T>
union v4_t {
	struct {
		T x;
		T y;
		T z;
		T w;
	};
	struct {
		T x1;
		T y1;
		T x2;
		T y2;
	};
	struct {
		T r;
		T g;
		T b;
		T a;
	};
	struct {
		v3_t<T> xyz;
		T __ignore1;
	};
	struct {
		v3_t<T> rgb;
		T __ignore2;
	};
	struct {
		v2_t<T> xy;
		v2_t<T> zw;
	};
	struct {
		T __ignore3;
		v2_t<T> yz;
		T __ignore4;
	};
	T f[4] = {};
	v4_t() {};
};
typedef v4_t<f32> v4;
typedef v4_t<i32> iv4;
typedef v4_t<u32> uv4;
typedef v4_t<u8>  bv4;
typedef bv4 	  color;
typedef v4 		  colorf;
template union v4_t<f32>;
template union v4_t<i32>;
template union v4_t<u32>;
template union v4_t<u8>;

union m4 {
	f32 v[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; 
	f32 f[4][4]; // column row
	struct {
		f32 _11, _21, _31, _41;
		f32 _12, _22, _32, _42;
		f32 _13, _23, _33, _43;
		f32 _14, _24, _34, _44;
	};
};

inline u32 mod(u32 dividend, u32 devisor);
inline f32 _sqrtf(f32 value);
inline f32 _tanf(f32 value);
inline i32 _abs(i32 value);
inline f32 absf_(f32 value);
inline f64 absf_(f64 value);
inline f32 _roundf(f32 value);
inline f32 _ceilf(f32 value);
inline i32 _floorf(f32 value);
inline f32 _sinf(f32 value);
inline f32 _cosf(f32 value);
inline f32 _lerpf(f32 min, f32 max, f32 dist);
inline u32 last_pow_two(u32 val);
inline u32 next_pow_two(u32 val);
template<typename T> inline v2_t<T> V2(T x, T y);
template<typename T> inline v2 V2f(T x, T y);
template<typename T> inline uv2 V2u(T x, T y);
template<typename T> inline v3_t<T> V3(T x, T y, T z);
template<typename T> inline v3 V3f(T x, T y, T z);
template<typename T> inline uv3 V3u(T x, T y, T z);
template<typename T> inline iv3 V3i(T x, T y, T z);
template<typename T> inline bv3 V3b(T x, T y, T z);
template<typename T> inline v4_t<T> V4(T x, T y, T z, T w);
template<typename T> inline bv4 V4b(T x, T y, T z, T w);
template<typename T> inline bv4 V4b(bv3 vec3, T w);
inline v4 color_to_f(color c);
template<typename T> inline T clamp(T V, T min, T max);
template<typename T> inline v2_t<T> clamp(v2_t<T> V, T min, T max);
template<typename T> inline v3_t<T> clamp(v3_t<T> V, T min, T max);
template<typename T> inline v4_t<T> clamp(v4_t<T> V, T min, T max);
template<typename T> inline T lengthsq(v2_t<T> V);
template<typename T> inline T lengthsq(v3_t<T> V);
template<typename T> inline T lengthsq(v4_t<T> V);
template<typename T> inline T length(v2_t<T> V);
template<typename T> inline T length(v3_t<T> V);
template<typename T> inline T length(v4_t<T> V);
template<typename T> inline v2_t<T> normalize(v2_t<T> V);
template<typename T> inline v3_t<T> normalize(v3_t<T> V);
template<typename T> inline v4_t<T> normalize(v4_t<T> V);
template<typename T> inline T dot(v2_t<T> l, v2_t<T> r);
template<typename T> inline T dot(v3_t<T> l, v3_t<T> r);
template<typename T> inline T dot(v4_t<T> l, v4_t<T> r);
template<typename T> inline T operator*(v2_t<T> l, v2_t<T> r);
template<typename T> inline T operator*(v3_t<T> l, v3_t<T> r);
template<typename T> inline T operator*(v4_t<T> l, v4_t<T> r);
template<typename T> inline v3_t<T> cross(v3_t<T> l, v3_t<T> r);
template<typename T> inline v2_t<T> add(v2_t<T> l, v2_t<T> r);
template<typename T> inline v3_t<T> add(v3_t<T> l, v3_t<T> r);
template<typename T> inline v4_t<T> add(v4_t<T> l, v4_t<T> r);
template<typename T> inline v2_t<T> operator+(v2_t<T> l, v2_t<T> r);
template<typename T> inline v3_t<T> operator+(v3_t<T> l, v3_t<T> r);
template<typename T> inline v4_t<T> operator+(v4_t<T> l, v4_t<T> r);
template<typename T> inline v2_t<T> sub(v2_t<T> l, v2_t<T> r);
template<typename T> inline v3_t<T> sub(v3_t<T> l, v3_t<T> r);
template<typename T> inline v4_t<T> sub(v4_t<T> l, v4_t<T> r);
template<typename T> inline v2_t<T> operator-(v2_t<T> l, v2_t<T> r);
template<typename T> inline v3_t<T> operator-(v3_t<T> l, v3_t<T> r);
template<typename T> inline v4_t<T> operator-(v4_t<T> l, v4_t<T> r);
template<typename T> inline v2_t<T> mult(v2_t<T> l, v2_t<T> r);
template<typename T> inline v2_t<T> mult(v2_t<T> l, T r);
template<typename T> inline v3_t<T> mult(v3_t<T> l, v3_t<T> r);
template<typename T> inline v3_t<T> mult(v3_t<T> l, T r);
template<typename T> inline v4_t<T> mult(v4_t<T> l, v4_t<T> r);
template<typename T> inline v4_t<T> mult(v4_t<T> l, T r);
template<typename T> inline v2_t<T> operator*(v2_t<T> l, T r);
template<typename T> inline v3_t<T> operator*(v3_t<T> l, T r);
template<typename T> inline v4_t<T> operator*(v4_t<T> l, T r);
template<typename T> inline v2_t<T> div(v2_t<T> l, v2_t<T> r);
template<typename T> inline v2_t<T> div(v2_t<T> l, T r);
template<typename T> inline v3_t<T> div(v3_t<T> l, v3_t<T> r);
template<typename T> inline v3_t<T> div(v3_t<T> l, T r);
template<typename T> inline v4_t<T> div(v4_t<T> l, v4_t<T> r);
template<typename T> inline v4_t<T> div(v4_t<T> l, T r);
template<typename T> inline v2_t<T> operator/(v2_t<T> l, T r);
template<typename T> inline v3_t<T> operator/(v3_t<T> l, T r);
template<typename T> inline v4_t<T> operator/(v4_t<T> l, T r);
template<typename T> inline r2_t<T> R2(T x, T y, T w, T h);
r2 R2f(i32 x, i32 y, i32 w, i32 h);
ur2 roundR2(r2 r);
template<typename T> inline r2_t<T> R2(v2_t<T> xy, v2_t<T> wh);
template<typename T> inline r2_t<T> add(r2_t<T> l, r2_t<T> r);
template<typename T> inline r2_t<T> add(r2_t<T> l, v2_t<T> r);
template<typename T> inline r2_t<T> operator+(r2_t<T> l, r2_t<T> r);
template<typename T> inline r2_t<T> operator+(r2_t<T> l, v2_t<T> r);
template<typename T> inline r2_t<T> sub(r2_t<T> l, r2_t<T> r);
template<typename T> inline r2_t<T> sub(r2_t<T> l, v2_t<T> r);
template<typename T> inline r2_t<T> operator-(r2_t<T> l, r2_t<T> r);
template<typename T> inline r2_t<T> operator-(r2_t<T> l, v2_t<T> r);
template<typename T> inline r2_t<T> mult(r2_t<T> r, T f);
template<typename T> inline r2_t<T> operator*(r2_t<T> r, T f);
template<typename T> inline bool intersect(r2_t<T> r, r2_t<T> l);
template<typename T> inline bool inside(r2_t<T> r, T x, T y);
template<typename T> inline bool inside(r2_t<T> r, v2_t<T> v);

m4 M4D(f32 diag);
m4 add(m4 l, m4 r);
m4 operator+(m4 l, m4 r);
m4 sub(m4 l, m4 r);
m4 operator-(m4 l, m4 r);
m4 mult(m4 l, m4 r);
m4 operator*(m4 l, m4 r);
m4 mult(m4 m, f32 s);
v4 mult(m4 m, v4 v);
m4 operator*(m4 m, f32 s);
v4 operator*(m4 m, v4 v);
m4 div(m4 m, f32 s);
m4 operator/(m4 m, f32 s);
m4 transpose(m4 m);
m4 ortho(f32 left, f32 right, f32 bot, f32 top, f32 near, f32 far);
m4 proj(f32 fov, f32 ar, f32 near, f32 far);
m4 translate(v3 trans);
m4 rotate(f32 angle, v3 axis);
m4 scale(v3 scale);
m4 lookAt(v3 eye, v3 center, v3 up);

const color WHITE = V4b(255, 255, 255, 255);
const color BLACK = V4b(0, 0, 0, 255);
const color RED   = V4b(255, 0, 0, 255);
const color GREEN = V4b(0, 255, 0, 255);
const color BLUE  = V4b(0, 0, 255, 255);

// From stb_perlin
f32 perlin(f32 x, f32 y, f32 z, i32 x_wrap, i32 y_wrap, i32 z_wrap);
f32 perlin_grad(i32 hash, f32 x, f32 y, f32 z);
