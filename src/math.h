#pragma once

// TODO(max): remove cmath, use more SIMD intrinsics
	// apply to v2_t, v3_t, v4_t, m4_t ops
		// use for packed normal math ops
	// make v2_t_4x, v3_t_4x, v4_t_4x, m4_t_4x
	// mod, div, sign, trig, bit-scanning
// TODO(max): quaternions
#include <cmath>
#include <xmmintrin.h>

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
		T z;
	};
	struct {
		T x;
		v2_t<T> yz;
	};
	T f[3] = {};
	v3_t() {};
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
		T w;
	};
	struct {
		v3_t<T> rgb;
		T a;
	};
	struct {
		v2_t<T> xy;
		v2_t<T> zw;
	};
	struct {
		T x;
		v2_t<T> yz;
		T w;
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

template<typename T>
union m4_t {
	T v[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; 
	T f[4][4]; // column row
	struct {
		T _11, _21, _31, _41;
		T _12, _22, _32, _42;
		T _13, _23, _33, _43;
		T _14, _24, _34, _44;
	};
};
typedef m4_t<f32> m4;
typedef m4_t<u32> um4;
template union m4_t<f32>;
template union m4_t<u32>;

inline u32 mod(u32 dividend, u32 devisor);
inline f32 _sqrtf(f32 value);
inline f32 _tanf(f32 value);
inline i32 _abs(i32 value);
inline f32 absf_(f32 value);
inline f64 absf_(f64 value);
inline f32 _roundf(f32 value);
inline f32 _ceilf(f32 value);
inline f32 _floorf(f32 value);
inline f32 _sinf(f32 value);
inline f32 _cosf(f32 value);
inline f32 _lerpf(f32 min, f32 max, f32 dist);
template<typename T> inline v2_t<T> V2(T x, T y);
template<typename T> inline v2 V2f(T x, T y);
template<typename T> inline uv2 V2u(T x, T y);
template<typename T> inline v3_t<T> V3(T x, T y, T z);
template<typename T> inline v3 V3f(T x, T y, T z);
template<typename T> inline uv3 V3u(T x, T y, T z);
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
template<typename T> inline v3_t<T> cross(v3_t<T> l, v3_t<T> r);
template<typename T> inline v2_t<T> add(v2_t<T> l, v2_t<T> r);
template<typename T> inline v3_t<T> add(v3_t<T> l, v3_t<T> r);
template<typename T> inline v4_t<T> add(v4_t<T> l, v4_t<T> r);
template<typename T> inline v2_t<T> sub(v2_t<T> l, v2_t<T> r);
template<typename T> inline v3_t<T> sub(v3_t<T> l, v3_t<T> r);
template<typename T> inline v4_t<T> sub(v4_t<T> l, v4_t<T> r);
template<typename T> inline v2_t<T> mult(v2_t<T> l, v2_t<T> r);
template<typename T> inline v2_t<T> mult(v2_t<T> l, T r);
template<typename T> inline v3_t<T> mult(v3_t<T> l, v3_t<T> r);
template<typename T> inline v3_t<T> mult(v3_t<T> l, T r);
template<typename T> inline v4_t<T> mult(v4_t<T> l, v4_t<T> r);
template<typename T> inline v4_t<T> mult(v4_t<T> l, T r);
template<typename T> inline v2_t<T> div(v2_t<T> l, v2_t<T> r);
template<typename T> inline v2_t<T> div(v2_t<T> l, T r);
template<typename T> inline v3_t<T> div(v3_t<T> l, v3_t<T> r);
template<typename T> inline v3_t<T> div(v3_t<T> l, T r);
template<typename T> inline v4_t<T> div(v4_t<T> l, v4_t<T> r);
template<typename T> inline v4_t<T> div(v4_t<T> l, T r);
template<typename T> inline r2_t<T> R2(T x, T y, T w, T h);
r2 R2f(i32 x, i32 y, i32 w, i32 h);
template<typename T> inline r2_t<T> R2(v2_t<T> xy, v2_t<T> wh);
template<typename T> inline r2_t<T> add(r2_t<T> l, r2_t<T> r);
template<typename T> inline r2_t<T> sub(r2_t<T> l, r2_t<T> r);
template<typename T> inline r2_t<T> mult(r2_t<T> r, T f);
template<typename T> inline bool inside(r2_t<T> r, T x, T y);
template<typename T> inline bool inside(r2_t<T> r, v2_t<T> v);
template<typename T> inline m4_t<T> M4D(T diag);
template<typename T> inline m4_t<T> add(m4_t<T> l, m4_t<T> r);
template<typename T> inline m4_t<T> sub(m4_t<T> l, m4_t<T> r);
template<typename T> m4_t<T> mult(m4_t<T> l, m4_t<T> r);
template<> inline m4 mult(m4 l, m4 r);
template<typename T> inline m4_t<T> mult(m4_t<T> m, T s);
template<typename T> inline v4_t<T> mult(m4_t<T> m, v4_t<T> v);
template<typename T> inline m4_t<T> div(m4_t<T> m, T s);
template<typename T> inline m4_t<T> transpose(m4_t<T> m);
inline m4 ortho(f32 left, f32 right, f32 bot, f32 top, f32 _near, f32 _far);
inline m4 proj(f32 fov, f32 ar, f32 _near, f32 _far);
inline m4 translate(v3 trans);
inline m4 rotate(f32 angle, v3 axis);
inline m4 scale(v3 scale);
inline m4 lookAt(v3 eye, v3 center, v3 up);
