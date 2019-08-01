
#define IN_MATH_IMPL
#include "math.h"

v3 iv3::to_f() {return v3(x, y, z);}

m4 m4::zero = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
m4 m4::I 	= {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

// yes most of these are useless... TODO(max): improve on math.h functions

u32 mod(u32 l, u32 r) {
	return l % r;
}
u64 mod(u64 l, u64 r) {
	return l % r;
}
f32 mod(f32 l, f32 r) {
	return fmodf(l,r);
}
f64 mod(f64 l, f64 r) {
	return fmod(l,r);
}
f32 _pow(f32 l, f32 r) {
	return powf(l,r);
}
f64 _pow(f64 l, f64 r) {
	return pow(l,r);
}
f32 fract(f32 v) {
	return v - (i64)v;
}
f64 fract(f64 v) {
	return v - (i64)v;
}
f32 _sqrt(f32 v) {
	return sqrtf(v);
}
f32 _absv(f32 v) {
	return v >= 0.0f ? v : -v;
}
f64 _absv(f64 v) {
	return v >= 0.0f ? v : -v;
}
f32 _round(f32 v) {
	return roundf(v);
}
f32 _ceil(f32 v) {
	return ceilf(v);
}
f32 _floor(f32 v) {
	return floorf(v);
}
f32 _sin(f32 v) {
	return sinf(v);
}
f32 _cos(f32 v) {
	return cosf(v);
}
f32 _tan(f32 v) {
	return tanf(v);
}
f32 _asin(f32 v) {
	return asinf(v);
}
f32 _acos(f32 v) {
	return acosf(v);
}
f32 _atan(f32 v) {
	return atanf(v);
}
f32 _atan2(f32 l, f32 r) {
	return atan2f(l, r);
}
f32 lerp(f32 min, f32 max, f32 dist) {
	return (max - min) * dist + min;
}
f32 clamp(f32 val, f32 min, f32 max) {
	if(val < min) return min;
	if(val > max) return max;
	return val;
}
u32 last_pow_two(u32 val) {

	u32 pos = 0;
#ifdef _MSC_VER
	_BitScanReverse((unsigned long*)&pos, val);
#else
    for(u32 bit = 31; bit >= 0; bit--) {
        if(val & (1 << bit)) {
            pos = bit;
            break;
        }
    }
#endif
	return 1 << pos;
}
u32 next_pow_two(u32 val) {

	return last_pow_two(val) << 1;
}

f32 smoothstep(f32 e0, f32 e1, f32 x) {
	f32 t = clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

f32 perlin(f32 x, f32 y, f32 z, i32 x_wrap, i32 y_wrap, i32 z_wrap) {
	static u8 perlin_random[512] = {
	   23, 125, 161, 52, 103, 117, 70, 37, 247, 101, 203, 169, 124, 126, 44, 123, 
	   152, 238, 145, 45, 171, 114, 253, 10, 192, 136, 4, 157, 249, 30, 35, 72, 
	   175, 63, 77, 90, 181, 16, 96, 111, 133, 104, 75, 162, 93, 56, 66, 240, 
	   8, 50, 84, 229, 49, 210, 173, 239, 141, 1, 87, 18, 2, 198, 143, 57, 
	   225, 160, 58, 217, 168, 206, 245, 204, 199, 6, 73, 60, 20, 230, 211, 233, 
	   94, 200, 88, 9, 74, 155, 33, 15, 219, 130, 226, 202, 83, 236, 42, 172, 
	   165, 218, 55, 222, 46, 107, 98, 154, 109, 67, 196, 178, 127, 158, 13, 243, 
	   65, 79, 166, 248, 25, 224, 115, 80, 68, 51, 184, 128, 232, 208, 151, 122, 
	   26, 212, 105, 43, 179, 213, 235, 148, 146, 89, 14, 195, 28, 78, 112, 76, 
	   250, 47, 24, 251, 140, 108, 186, 190, 228, 170, 183, 139, 39, 188, 244, 246, 
	   132, 48, 119, 144, 180, 138, 134, 193, 82, 182, 120, 121, 86, 220, 209, 3, 
	   91, 241, 149, 85, 205, 150, 113, 216, 31, 100, 41, 164, 177, 214, 153, 231, 
	   38, 71, 185, 174, 97, 201, 29, 95, 7, 92, 54, 254, 191, 118, 34, 221, 
	   131, 11, 163, 99, 234, 81, 227, 147, 156, 176, 17, 142, 69, 12, 110, 62, 
	   27, 255, 0, 194, 59, 116, 242, 252, 19, 21, 187, 53, 207, 129, 64, 135, 
	   61, 40, 167, 237, 102, 223, 106, 159, 197, 189, 215, 137, 36, 32, 22, 5,  
	   23, 125, 161, 52, 103, 117, 70, 37, 247, 101, 203, 169, 124, 126, 44, 123, 
	   152, 238, 145, 45, 171, 114, 253, 10, 192, 136, 4, 157, 249, 30, 35, 72, 
	   175, 63, 77, 90, 181, 16, 96, 111, 133, 104, 75, 162, 93, 56, 66, 240, 
	   8, 50, 84, 229, 49, 210, 173, 239, 141, 1, 87, 18, 2, 198, 143, 57, 
	   225, 160, 58, 217, 168, 206, 245, 204, 199, 6, 73, 60, 20, 230, 211, 233, 
	   94, 200, 88, 9, 74, 155, 33, 15, 219, 130, 226, 202, 83, 236, 42, 172, 
	   165, 218, 55, 222, 46, 107, 98, 154, 109, 67, 196, 178, 127, 158, 13, 243, 
	   65, 79, 166, 248, 25, 224, 115, 80, 68, 51, 184, 128, 232, 208, 151, 122, 
	   26, 212, 105, 43, 179, 213, 235, 148, 146, 89, 14, 195, 28, 78, 112, 76, 
	   250, 47, 24, 251, 140, 108, 186, 190, 228, 170, 183, 139, 39, 188, 244, 246, 
	   132, 48, 119, 144, 180, 138, 134, 193, 82, 182, 120, 121, 86, 220, 209, 3, 
	   91, 241, 149, 85, 205, 150, 113, 216, 31, 100, 41, 164, 177, 214, 153, 231, 
	   38, 71, 185, 174, 97, 201, 29, 95, 7, 92, 54, 254, 191, 118, 34, 221, 
	   131, 11, 163, 99, 234, 81, 227, 147, 156, 176, 17, 142, 69, 12, 110, 62, 
	   27, 255, 0, 194, 59, 116, 242, 252, 19, 21, 187, 53, 207, 129, 64, 135, 
	   61, 40, 167, 237, 102, 223, 106, 159, 197, 189, 215, 137, 36, 32, 22, 5,  
	};

   f32 u,v,w;
   f32 n000,n001,n010,n011,n100,n101,n110,n111;
   f32 n00,n01,n10,n11;
   f32 n0,n1;

   u32 x_mask = (x_wrap-1) & 255;
   u32 y_mask = (y_wrap-1) & 255;
   u32 z_mask = (z_wrap-1) & 255;
   i32 px = (i32)floor(x);
   i32 py = (i32)floor(y);
   i32 pz = (i32)floor(z);
   i32 x0 = px & x_mask, x1 = (px+1) & x_mask;
   i32 y0 = py & y_mask, y1 = (py+1) & y_mask;
   i32 z0 = pz & z_mask, z1 = (pz+1) & z_mask;
   i32 r0,r1,r00,r01,r10,r11;

   #define ease(a) (((a*6-15)*a + 10) * a * a * a)

   x -= px; u = ease(x);
   y -= py; v = ease(y);
   z -= pz; w = ease(z);

   #undef ease

   r0 = perlin_random[x0];
   r1 = perlin_random[x1];

   r00 = perlin_random[r0+y0];
   r01 = perlin_random[r0+y1];
   r10 = perlin_random[r1+y0];
   r11 = perlin_random[r1+y1];

   n000 = perlin_grad(perlin_random[r00+z0], x  , y  , z   );
   n001 = perlin_grad(perlin_random[r00+z1], x  , y  , z-1 );
   n010 = perlin_grad(perlin_random[r01+z0], x  , y-1, z   );
   n011 = perlin_grad(perlin_random[r01+z1], x  , y-1, z-1 );
   n100 = perlin_grad(perlin_random[r10+z0], x-1, y  , z   );
   n101 = perlin_grad(perlin_random[r10+z1], x-1, y  , z-1 );
   n110 = perlin_grad(perlin_random[r11+z0], x-1, y-1, z   );
   n111 = perlin_grad(perlin_random[r11+z1], x-1, y-1, z-1 );

   n00 = lerp(n000,n001,w);
   n01 = lerp(n010,n011,w);
   n10 = lerp(n100,n101,w);
   n11 = lerp(n110,n111,w);

   n0 = lerp(n00,n01,v);
   n1 = lerp(n10,n11,v);

   return lerp(n0,n1,u);
}
f32 perlin_grad(i32 hash, f32 x, f32 y, f32 z) {
   static f32 basis[12][3] = {
      { 1, 1, 0},
      {-1, 1, 0},
      { 1,-1, 0},
      {-1,-1, 0},
      { 1, 0, 1},
      {-1, 0, 1},
      { 1, 0,-1},
      {-1, 0,-1},
      { 0, 1, 1},
      { 0,-1, 1},
      { 0, 1,-1},
      { 0,-1,-1},
   };
   static u8 indices[64] = {
      0,1,2,3,4,5,6,7,8,9,10,11,
      0,9,1,11,
      0,1,2,3,4,5,6,7,8,9,10,11,
      0,1,2,3,4,5,6,7,8,9,10,11,
      0,1,2,3,4,5,6,7,8,9,10,11,
      0,1,2,3,4,5,6,7,8,9,10,11,
   };

   f32 *grad = basis[indices[hash & 63]];
   return grad[0]*x + grad[1]*y + grad[2]*z;
}

#define _PS_CONST_TYPE(Name, Type, Val) static const __declspec(align(16)) Type _ps_##Name[4] = { Val, Val, Val, Val }
#define _PS_CONST(Name, Val) static const __declspec(align(16)) f32 _ps_##Name[4] = { Val, Val, Val, Val }
#define _PI32_CONST(Name, Val) static const __declspec(align(16)) int _pi32_##Name[4] = { Val, Val, Val, Val }
_PS_CONST_TYPE(sign_mask, int, (int)0x80000000);
_PS_CONST_TYPE(inv_sign_mask, int, ~0x80000000);
_PI32_CONST(1, 1);
_PI32_CONST(inv1, ~1);
_PI32_CONST(2, 2);
_PI32_CONST(4, 4);
_PI32_CONST(0x7f, 0x7f);
_PS_CONST(0p5, 0.5f);
_PS_CONST(1  , 1.0f);
_PS_CONST(cephes_FOPI, 1.27323954473516f);
_PS_CONST(minus_cephes_DP1, -0.78515625f);
_PS_CONST(minus_cephes_DP2, -2.4187564849853515625e-4f);
_PS_CONST(minus_cephes_DP3, -3.77489497744594108e-8f);
_PS_CONST(coscof_p0,  2.443315711809948E-005f);
_PS_CONST(coscof_p1, -1.388731625493765E-003f);
_PS_CONST(coscof_p2,  4.166664568298827E-002f);
_PS_CONST(sincof_p0, -1.9515295891E-4f);
_PS_CONST(sincof_p1,  8.3321608736E-3f);
_PS_CONST(sincof_p2, -1.6666654611E-1f);

v4 _sqrt(v4 v) {
	return {_mm_sqrt_ps(v.packed)};
}
v4 step(v4 l, v4 r) {
	v4 ret;
	for(i32 i = 0; i < 4; i++)
		ret[i] = l[i] < r[i];
	return ret;
}
v4 fract(v4 v) {
	return {_mm_sub_ps(v.packed, _mm_round_ps(v.packed, _MM_FROUND_TRUNC))};
}
v3 _absv(v3 v) {
	return {_absv(v.x),_absv(v.y),_absv(v.z)};
}
v4 _absv(v4 v) {
	// for some reason there is no _mm_abs_ps()
	return {_absv(v.x),_absv(v.y),_absv(v.z),_absv(v.w)};
}
v4 _round(v4 v) {
	return {_mm_round_ps(v.packed, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)};
}
v4 _ceil(v4 v) {
	return {_mm_round_ps(v.packed, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC)};
}
v4 _floor(v4 v) {
	return {_mm_round_ps(v.packed, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC)};
}
v4 _sin(v4 v) {
	__m128 xmm1, xmm2 = _mm_setzero_ps(), xmm3, sign_bit, y;

	__m128i emm0, emm2;
	sign_bit = v.packed;
	v.packed = _mm_and_ps(v.packed, *(__m128*)_ps_inv_sign_mask);
	sign_bit = _mm_and_ps(sign_bit, *(__m128*)_ps_sign_mask);

	y = _mm_mul_ps(v.packed, *(__m128*)_ps_cephes_FOPI);

	emm2 = _mm_cvttps_epi32(y);
	emm2 = _mm_add_epi32(emm2, *(__m128i*)_pi32_1);
	emm2 = _mm_and_si128(emm2, *(__m128i*)_pi32_inv1);
	y = _mm_cvtepi32_ps(emm2);

	emm0 = _mm_and_si128(emm2, *(__m128i*)_pi32_4);
	emm0 = _mm_slli_epi32(emm0, 29);
	emm2 = _mm_and_si128(emm2, *(__m128i*)_pi32_2);
	emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());

	__m128 swap_sign_bit = _mm_castsi128_ps(emm0);
	__m128 poly_mask = _mm_castsi128_ps(emm2);
	sign_bit = _mm_xor_ps(sign_bit, swap_sign_bit);

	xmm1 = *(__m128*)_ps_minus_cephes_DP1;
	xmm2 = *(__m128*)_ps_minus_cephes_DP2;
	xmm3 = *(__m128*)_ps_minus_cephes_DP3;
	xmm1 = _mm_mul_ps(y, xmm1);
	xmm2 = _mm_mul_ps(y, xmm2);
	xmm3 = _mm_mul_ps(y, xmm3);
	v.packed = _mm_add_ps(v.packed, xmm1);
	v.packed = _mm_add_ps(v.packed, xmm2);
	v.packed = _mm_add_ps(v.packed, xmm3);

	y = *(__m128*)_ps_coscof_p0;
	__m128 z = _mm_mul_ps(v.packed,v.packed);

	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, *(__m128*)_ps_coscof_p1);
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, *(__m128*)_ps_coscof_p2);
	y = _mm_mul_ps(y, z);
	y = _mm_mul_ps(y, z);
	__m128 tmp = _mm_mul_ps(z, *(__m128*)_ps_0p5);
	y = _mm_sub_ps(y, tmp);
	y = _mm_add_ps(y, *(__m128*)_ps_1);

	__m128 y2 = *(__m128*)_ps_sincof_p0;
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, *(__m128*)_ps_sincof_p1);
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, *(__m128*)_ps_sincof_p2);
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_mul_ps(y2, v.packed);
	y2 = _mm_add_ps(y2, v.packed);

	xmm3 = poly_mask;
	y2 = _mm_and_ps(xmm3, y2);
	y = _mm_andnot_ps(xmm3, y);
	y = _mm_add_ps(y,y2);
	y = _mm_xor_ps(y, sign_bit);
	return y;
}
v4 _cos(v4 v) {
	__m128 xmm1, xmm2 = _mm_setzero_ps(), xmm3, y;
	__m128i emm0, emm2;
	v.packed = _mm_and_ps(v.packed, *(__m128*)_ps_inv_sign_mask);
	y = _mm_mul_ps(v.packed, *(__m128*)_ps_cephes_FOPI);

	emm2 = _mm_cvttps_epi32(y);
	emm2 = _mm_add_epi32(emm2, *(__m128i*)_pi32_1);
	emm2 = _mm_and_si128(emm2, *(__m128i*)_pi32_inv1);
	y = _mm_cvtepi32_ps(emm2);

	emm2 = _mm_sub_epi32(emm2, *(__m128i*)_pi32_2);

	emm0 = _mm_andnot_si128(emm2, *(__m128i*)_pi32_4);
	emm0 = _mm_slli_epi32(emm0, 29);
	emm2 = _mm_and_si128(emm2, *(__m128i*)_pi32_2);
	emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());

	__m128 sign_bit = _mm_castsi128_ps(emm0);
	__m128 poly_mask = _mm_castsi128_ps(emm2);

	xmm1 = *(__m128*)_ps_minus_cephes_DP1;
	xmm2 = *(__m128*)_ps_minus_cephes_DP2;
	xmm3 = *(__m128*)_ps_minus_cephes_DP3;
	xmm1 = _mm_mul_ps(y, xmm1);
	xmm2 = _mm_mul_ps(y, xmm2);
	xmm3 = _mm_mul_ps(y, xmm3);
	v.packed = _mm_add_ps(v.packed, xmm1);
	v.packed = _mm_add_ps(v.packed, xmm2);
	v.packed = _mm_add_ps(v.packed, xmm3);

	y = *(__m128*)_ps_coscof_p0;
	__m128 z = _mm_mul_ps(v.packed,v.packed);

	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, *(__m128*)_ps_coscof_p1);
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, *(__m128*)_ps_coscof_p2);
	y = _mm_mul_ps(y, z);
	y = _mm_mul_ps(y, z);
	__m128 tmp = _mm_mul_ps(z, *(__m128*)_ps_0p5);
	y = _mm_sub_ps(y, tmp);
	y = _mm_add_ps(y, *(__m128*)_ps_1);

	__m128 y2 = *(__m128*)_ps_sincof_p0;
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, *(__m128*)_ps_sincof_p1);
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, *(__m128*)_ps_sincof_p2);
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_mul_ps(y2, v.packed);
	y2 = _mm_add_ps(y2, v.packed);

	xmm3 = poly_mask;
	y2 = _mm_and_ps(xmm3, y2);
	y = _mm_andnot_ps(xmm3, y);
	y = _mm_add_ps(y,y2);
	y = _mm_xor_ps(y, sign_bit);

	return y;
}
v4 lerp(v4 min, v4 max, v4 dist) {
	return {_mm_add_ps(_mm_mul_ps(_mm_sub_ps(max.packed, min.packed), dist.packed), min.packed)};
}
v4 clamp(v4 val, v4 min, v4 max) {
	__m128 _min = _mm_min_ps(val.packed, max.packed);
	return {_mm_max_ps(_min, min.packed)};
}

f32 lensq(v2 v) {
	return dot(v, v);
}
f32 lensq(v3 v) {
	return dot(v, v);
}
f32 lensq(v4 v) {
	return dot(v, v);
}
f32 len(v2 v) {
	return _sqrt(lensq(v));
}
f32 len(v3 v) {
	return _sqrt(lensq(v));
}
f32 len(v4 v) {
	return _sqrt(lensq(v));
}

bool all_less(v3 v, f32 c) {
	return v.x < c && v.y < c && v.z < c;
}
v2 minv2(v2 l, v2 r) {
	return v2(min(l.x,r.x),min(l.y,r.y));
}
v2 maxv2(v2 l, v2 r) {
	return v2(max(l.x,r.x),max(l.y,r.y));
}

f32 min_reset(v3 v) {
	if(isinf(v.x) || isnan(v.x)) v.x = 0.0f;
	if(isinf(v.y) || isnan(v.x)) v.y = 0.0f;
	if(isinf(v.z) || isnan(v.x)) v.z = 0.0f;
	return min3(v.x,v.y,v.z);
}

v2 norm(v2 v) {
	return v / len(v);
}
v3 norm(v3 v) {
	return v / len(v);
}
v4 norm(v4 v) {
	return v / len(v);
}

v3 reflect(v3 i, v3 n) {
	return i - 2.0f * dot(n, i) * n;
}

f32 dot(v2 l, v2 r) {
	return l.x * r.x + l.y * r.y;
}
f32 dot(v3 l, v3 r) {
	return l.x * r.x + l.y * r.y + l.z * r.z;
}
f32 dot(v4 l, v4 r) {
	return v4{_mm_dp_ps(l.packed, r.packed, 0xf1)}.x;
}
v3 cross(v3 l, v3 r) {
	return {l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x};
}
v2 operator+(v2 l, v2 r) {
	return {l.x + r.x, l.y + r.y};
}
v3 operator+(v3 l, v3 r) {
	return {l.x + r.x, l.y + r.y, l.z + r.z};	
}
v4 operator+(v4 l, v4 r) {
	return {_mm_add_ps(l.packed, r.packed)};
}
uv2 operator+(uv2 l, uv2 r) {
	return {l.x + r.x, l.y + r.y};
}

iv2 operator+(iv2 l, iv2 r) {
	return {l.x + r.x, l.y + r.y};
}
iv2 operator-(iv2 l, iv2 r) {
	return {l.x - r.x, l.y - r.y};
}
iv2 operator*(iv2 l, iv2 r) {
	return {l.x * r.x, l.y * r.y};
}
iv2 operator*(iv2 l, i32 r) {
	return {l.x * r, l.y * r};
}
iv2 operator*(i32 l, iv2 r) {
	return r * l;
}
iv2 operator/(iv2 l, iv2 r) {
	return {l.x / r.x, l.y / r.y};
}
iv2 operator/(iv2 l, i32 r) {
	return {l.x / r, l.y / r};
}
iv2 operator/(i32 l, iv2 r) {
	return r / l;
}

uv3 operator+(uv3 l, uv3 r) {
	return {l.x + r.x, l.y + r.y, l.z + r.z};
}
iv3 operator+(iv3 l, iv3 r) {
	return {l.x + r.x, l.y + r.y, l.z + r.z};
}
bv4 operator+(bv4 l, bv4 r) {
	return {(u8)(l.x + r.x), (u8)(l.y + r.y), (u8)(l.z + r.z), (u8)(l.w + r.w)};
}
v2 operator-(v2 l, v2 r) {
	return {l.x - r.x, l.y - r.y};
}
v3 operator-(v3 l, v3 r) {
	return {l.x - r.x, l.y - r.y, l.z - r.z};
}
v4 operator-(v4 l, v4 r) {
	return {_mm_sub_ps(l.packed, r.packed)};
}
uv2 operator-(uv2 l, uv2 r) {
	return {l.x - r.x, l.y - r.y};
}

uv3 operator-(uv3 l, uv3 r) {
	return {l.x - r.x, l.y - r.y, l.z - r.z};
}
iv3 operator-(iv3 l, iv3 r) {
	return {l.x - r.x, l.y - r.y, l.z - r.z};
}
bv4 operator-(bv4 l, bv4 r) {
	return {(u8)(l.x - r.x), (u8)(l.y - r.y), (u8)(l.z - r.z), (u8)(l.w - r.w)};
}
v2 operator*(v2 l, v2 r) {
	return {l.x * r.x, l.y * r.y};
}
v3 operator*(v3 l, v3 r) {
	return {l.x * r.x, l.y * r.y, l.z * r.z};
}
v4 operator*(v4 l, v4 r) {
	return {_mm_mul_ps(l.packed, r.packed)};
}
uv2 operator*(uv2 l, uv2 r) {
	return {l.x * r.x, l.y * r.y};
}
uv3 operator*(uv3 l, uv3 r) {
	return {l.x * r.x, l.y * r.y, l.z * r.z};
}
iv3 operator*(iv3 l, iv3 r) {
	return {l.x * r.x, l.y * r.y, l.z * r.z};	
}
bv4 operator*(bv4 l, bv4 r) {
	return {(u8)(l.x * r.x), (u8)(l.y * r.y), (u8)(l.z * r.z), (u8)(l.w * r.w)};
}
v2 operator*(v2 l, f32 r) {
	return {l.x * r, l.y * r};
}
v3 operator*(v3 l, f32 r) {
	return {l.x * r, l.y * r, l.z * r};
}
v3 operator*(v3 l, i32 r) {
	return {l.x * r, l.y * r, l.z * r};
}
v4 operator*(v4 l, f32 r) {
	return {_mm_mul_ps(l.packed, _mm_set1_ps(r))};
}
uv2 operator*(uv2 l, u32 r) {
	return {l.x * r, l.y * r};
}
uv3 operator*(uv3 l, u32 r) {
	return {l.x * r, l.y * r, l.z * r};
}
iv3 operator*(iv3 l, i32 r) {
	return {l.x * r, l.y * r, l.z * r};
}
v3 operator*(iv3 l, f32 r) {
	return {l.x * r, l.y * r, l.z * r};
}
bv4 operator*(bv4 l, u8 r) {
	return {(u8)(l.x * r), (u8)(l.y * r), (u8)(l.z * r), (u8)(l.w * r)};
}
v2 operator*(f32 l, v2 r) {
	return r * l;
}
v3 operator*(f32 l, v3 r) {
	return r * l;
}
v4 operator*(f32 l, v4 r) {
	return r * l;
}
uv2 operator*(u32 l, uv2 r) {
	return r * l;
}

uv3 operator*(u32 l, uv3 r) {
	return r * l;
}
iv3 operator*(i32 l, iv3 r) {
	return r * l;
}
bv4 operator*(u8 l, bv4 r) {
	return r * l;
}
v2 operator/(v2 l, v2 r) {
	return {l.x / r.x, l.y / r.y};
}
v3 operator/(v3 l, v3 r) {
	return {l.x / r.x, l.y / r.y, l.z / r.z};
}
v4 operator/(v4 l, v4 r) {
	return {_mm_div_ps(l.packed, r.packed)};
}
uv2 operator/(uv2 l, uv2 r) {
	return {l.x / r.x, l.y / r.y};
}

uv3 operator/(uv3 l, uv3 r) {
	return {l.x / r.x, l.y / r.y, l.z / r.z};
}
iv3 operator/(iv3 l, iv3 r) {
	return {l.x / r.x, l.y / r.y, l.z / r.z};
}
bv4 operator/(bv4 l, bv4 r) {
	return {(u8)(l.x * r.x), (u8)(l.y * r.y), (u8)(l.z * r.z), (u8)(l.w * r.w)};
}
v2 operator/(v2 l, f32 r) {
	return {l.x / r, l.y / r};
}
v3 operator/(v3 l, f32 r) {
	return {l.x / r, l.y / r, l.z / r};
}
v4 operator/(v4 l, f32 r) {
	return {_mm_div_ps(l.packed, _mm_set1_ps(r))};
}
uv2 operator/(uv2 l, u32 r) {
	return {l.x / r, l.y / r};
}

uv3 operator/(uv3 l, u32 r) {
	return {l.x / r, l.y / r, l.z / r};
}
iv3 operator/(iv3 l, i32 r) {
	return {l.x / r, l.y / r, l.z / r};
}
bv4 operator/(bv4 l, u8 r) {
	return {(u8)(l.x / r), (u8)(l.y / r), (u8)(l.z / r), (u8)(l.w / r)};
}
v2 operator/(f32 l, v2 r) {
	return r / l;
}
v3 operator/(f32 l, v3 r) {
	return r / l;
}
v4 operator/(f32 l, v4 r) {
	return r / l;
}
uv2 operator/(u32 l, uv2 r) {
	return r / l;
}

uv3 operator/(u32 l, uv3 r) {
	return r / l;
}
iv3 operator/(i32 l, iv3 r) {
	return r / l;
}
bv4 operator/(u8 l, bv4 r) {
	return r / l;
}

r2 operator+(r2 l, r2 r) {
	return {_mm_add_ps(l.packed, r.packed)};
}
r2 operator+(r2 l, v2 r) {
	return {l.xy + r, l.wh};
}
r2 operator-(r2 l, r2 r) {
	return {_mm_sub_ps(l.packed, r.packed)};
}
r2 operator-(r2 l, v2 r) {
	return {l.xy - r, l.wh};
}
r2 operator*(r2 l, f32 r) {
	return {l.xy, l.wh * r};
}
ur2 operator+(ur2 l, ur2 r) {
	return {l.xy + r.xy, l.wh + r.wh};
}
ur2 operator+(ur2 l, uv2 r) {
	return {l.xy + r, l.wh};
}
ur2 operator-(ur2 l, ur2 r) {
	return {l.xy - r.xy, l.wh - r.wh};
}
ur2 operator-(ur2 l, uv2 r) {
	return {l.xy - r, l.wh};
}
ur2 operator*(ur2 l, u32 r) {
	return {l.xy, l.wh * r};
}
ir2 operator+(ir2 l, ir2 r) {
	return {l.xy + r.xy, l.wh + r.wh};
}
ir2 operator+(ir2 l, iv2 r) {
	return {l.xy + r, l.wh};
}
ir2 operator-(ir2 l, ir2 r) {
	return {l.xy - r.xy, l.wh - r.wh};
}
ir2 operator-(ir2 l, iv2 r) {
	return {l.xy - r, l.wh};
}
ir2 operator*(ir2 l, i32 r) {
	return {l.xy, l.wh * r};
}

bool inside(r2 l, v2 r) {
	return (r.x >= l.x) && (r.x <= l.x + l.w) && (r.y >= l.y) && (r.y <= l.y + l.h);
}
bool intersect(r2 l, r2 r) {
	return (l.x <= r.x + r.w) && (l.x + l.w >= r.x) && (l.y <= r.y + r.h) && (l.y + l.h >= r.y);
}

bool operator==(v2 l, v2 r) {
	return l.x == r.x && l.y == r.y;
}
bool operator==(uv2 l, uv2 r) {
	return l.x == r.x && l.y == r.y;
}
bool operator==(v3 l, v3 r) {
	return l.x == r.x && l.y == r.y && l.z == r.z;
}
bool operator==(iv3 l, iv3 r) {
	return l.x == r.x && l.y == r.y && l.z == r.z;
}
bool operator==(uv3 l, uv3 r) {
	return l.x == r.x && l.y == r.y && l.z == r.z;
}
bool operator==(v4 l, v4 r) {
	__m128 cmp = _mm_cmpeq_ps(l.packed, r.packed);
	i32 test = _mm_movemask_ps(cmp);
	return test == 0x0000000f;
}
bool operator==(bv4 l, bv4 r) {
	return l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w;
}
bool operator==(m4 l, m4 r) {
	for(i32 i = 0; i < 4; i++)
		if(l.columns[i] != r.columns[i])
			return false;
	return true;
}
bool operator==(r2 l, r2 r) {
	__m128 cmp = _mm_cmpeq_ps(l.packed, r.packed);
	i32 test = _mm_movemask_ps(cmp);
	return test == 0x0000000f;
}
bool operator==(ur2 l, ur2 r) {
	return l.x == r.x && l.y == r.y && l.w == r.w && l.h == r.h;
}
bool operator==(ir2 l, ir2 r) {
	return l.x == r.x && l.y == r.y && l.w == r.w && l.h == r.h;
}
bool operator==(color l, color r) {
	return l.r == r.r && l.g == r.g && l.b == r.b && l.a == r.a;
}
bool operator==(color3 l, color3 r) {
	return l.r == r.r && l.g == r.g && l.b == r.b;
}
bool operator==(colorf l, colorf r) {
	return l.r == r.r && l.g == r.g && l.b == r.b && l.a == r.a;
}
bool operator!=(v2 l, v2 r) {
	return !(l == r);
}
bool operator!=(uv2 l, uv2 r) {
	return !(l == r);
}
bool operator!=(v3 l, v3 r) {
	return !(l == r);
}
bool operator!=(iv3 l, iv3 r) {
	return !(l == r);
}
bool operator!=(uv3 l, uv3 r) {
	return !(l == r);
}
bool operator!=(v4 l, v4 r) {
	return !(l == r);
}
bool operator!=(bv4 l, bv4 r) {
	return !(l == r);
}
bool operator!=(m4 l, m4 r) {
	return !(l == r);
}
bool operator!=(r2 l, r2 r) {
	return !(l == r);
}
bool operator!=(ur2 l, ur2 r) {
	return !(l == r);
}
bool operator!=(ir2 l, ir2 r) {
	return !(l == r);
}
bool operator!=(color l, color r) {
	return !(l == r);
}
bool operator!=(color3 l, color3 r) {
	return !(l == r);
}
bool operator!=(colorf l, colorf r) {
	return !(l == r);
}
bool fudge(f32 l, f32 r) {
	return (l >= r - EPSILON32) && (l <= r + EPSILON32);
}
bool fudge(v2 l, v2 r) {
	return fudge(l.x,r.x) && fudge(l.y,r.y);
}
bool fudge(v3 l, v3 r) {
	return fudge(l.x,r.x) && fudge(l.y,r.y) && fudge(l.z,r.z);
}
bool fudge(v4 l, v4 r) {
	return fudge(l.x,r.x) && fudge(l.y,r.y) && fudge(l.z,r.z) && fudge(l.w,r.w);
}

m4 operator+(m4 l, m4 r) {
	m4 ret;
	for(i32 i = 0; i < 4; i++)
		ret.packed[i] = _mm_add_ps(l.packed[i], r.packed[i]);
	return ret;
}
m4 operator-(m4 l, m4 r) {
	m4 ret;
	for(i32 i = 0; i < 4; i++)
		ret.packed[i] = _mm_sub_ps(l.packed[i], r.packed[i]);
	return ret;
}
m4 operator*(m4 l, m4 r) {
	m4 ret;
    for(i32 i = 0; i < 4; i++) {
        ret.packed[i] = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(r[i][0]), l.packed[0]),
            _mm_mul_ps(_mm_set1_ps(r[i][1]), l.packed[1])), 
       	_mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(r[i][2]), l.packed[2]),
            _mm_mul_ps(_mm_set1_ps(r[i][3]), l.packed[3])));
    }
    return ret;
}
v4 operator*(m4 l, v4 r) {
    v4 ret;
    m4 lt = transpose(l);
    for(i32 i = 0; i < 4; i++)
        ret[i] = dot(lt.columns[i], r);
    return ret;
}
m4 operator*(m4 l, f32 r) {
	m4 ret;
	for(i32 i = 0; i < 4; i++)
		ret.packed[i] = _mm_mul_ps(l.packed[i], _mm_set1_ps(r));
	return ret;
}
m4 operator/(m4 l, f32 r) {
	m4 ret;
	for(i32 i = 0; i < 4; i++)
		ret.packed[i] = _mm_div_ps(l.packed[i], _mm_set1_ps(r));
	return ret;
}
m4 transpose(m4 m) {
	m4 ret;
	for(i32 i = 0; i < 4; i++)
		for(u8 j = 0; j < 4; j++)
			ret[i][j] = m[j][i];
	return ret;
}
m4 ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    m4 ret;
    ret[0][0] = 2.0f / (r - l);
    ret[1][1] = 2.0f / (t - b);
    ret[2][2] = 2.0f / (n - f);
    ret[3][0] = (-l - r) / (r - l);
    ret[3][1] = (-b - t)  / (t - b);
    ret[3][2] = - n / (f - n);
    return ret;
}

m4 project(f32 fov, f32 ar, f32 n)
{
    float f = 1.0f / _tan(RADIANS(fov) / 2.0f);
    m4 ret;
    ret[0][0] = f / ar;
    ret[1][1] = f;
    ret[2][2] = 0.0f;
    ret[3][3] = 0.0f;
    ret[3][2] = n;
    ret[2][3] = -1.0f;
    return ret;
}
m4 translate(v3 v) {
	m4 ret;
	ret[3].xyz = v;
    return ret;
}
m4 rotate(f32 a, v3 axis) {
	m4 ret;
	f32 c = _cos(RADIANS(a));
	f32 s = _sin(RADIANS(a));
	
	axis = norm(axis);
	v3 temp = axis * (1.0f - c);

	ret[0][0] = c + temp[0] * axis[0];
	ret[0][1] = temp[0] * axis[1] + s * axis[2];
	ret[0][2] = temp[0] * axis[2] - s * axis[1];
	ret[1][0] = temp[1] * axis[0] - s * axis[2];
	ret[1][1] = c + temp[1] * axis[1];
	ret[1][2] = temp[1] * axis[2] + s * axis[0];
	ret[2][0] = temp[2] * axis[0] + s * axis[1];
	ret[2][1] = temp[2] * axis[1] - s * axis[0];
	ret[2][2] = c + temp[2] * axis[2];

	return ret;
}
m4 scale(v3 s) {
	m4 ret;
    ret[0][0] = s.x;
    ret[1][1] = s.y;
    ret[2][2] = s.z;
    return ret;
}
m4 lookAt(v3 pos, v3 at, v3 up) {
    m4 ret = m4::zero;

    v3 F = norm(at - pos);
    v3 S = norm(cross(F, up));
    v3 U = cross(S, F);

    ret[0][0] =  S.x;
    ret[0][1] =  U.x;
    ret[0][2] = -F.x;
    ret[1][0] =  S.y;
    ret[1][1] =  U.y;
    ret[1][2] = -F.y;
    ret[2][0] =  S.z;
    ret[2][1] =  U.z;
    ret[2][2] = -F.z;
    ret[3][0] = -dot(S, pos);
    ret[3][1] = -dot(U, pos);
    ret[3][2] =  dot(F, pos);
    ret[3][3] = 1.0f;

    return ret;
}


// SSE matrix inverse from https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

#define MakeShuffleMask(x,y,z,w)           (x | (y<<2) | (z<<4) | (w<<6))
#define VecSwizzleMask(vec, mask)          _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w)        VecSwizzleMask(vec, MakeShuffleMask(x,y,z,w))
#define VecSwizzle1(vec, x)                VecSwizzleMask(vec, MakeShuffleMask(x,x,x,x))
#define VecSwizzle_0022(vec)               _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec)               _mm_movehdup_ps(vec)
#define VecShuffle(vec1, vec2, x,y,z,w)    _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x,y,z,w))
#define VecShuffle_0101(vec1, vec2)        _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2)        _mm_movehl_ps(vec2, vec1)
inline __m128 Mat2Mul(__m128 vec1, __m128 vec2)
{
	return
		_mm_add_ps(_mm_mul_ps(                     vec1, VecSwizzle(vec2, 0,3,0,3)),
		           _mm_mul_ps(VecSwizzle(vec1, 1,0,3,2), VecSwizzle(vec2, 2,1,2,1)));
}
inline __m128 Mat2AdjMul(__m128 vec1, __m128 vec2)
{
	return
		_mm_sub_ps(_mm_mul_ps(VecSwizzle(vec1, 3,3,0,0), vec2),
		           _mm_mul_ps(VecSwizzle(vec1, 1,1,2,2), VecSwizzle(vec2, 2,3,0,1)));

}
inline __m128 Mat2MulAdj(__m128 vec1, __m128 vec2)
{
	return
		_mm_sub_ps(_mm_mul_ps(                     vec1, VecSwizzle(vec2, 3,0,3,0)),
		           _mm_mul_ps(VecSwizzle(vec1, 1,0,3,2), VecSwizzle(vec2, 2,1,2,1)));
}
m4 inverse(m4 inM)
{
	__m128 A = VecShuffle_0101(inM.packed[0], inM.packed[1]);
	__m128 B = VecShuffle_2323(inM.packed[0], inM.packed[1]);
	__m128 C = VecShuffle_0101(inM.packed[2], inM.packed[3]);
	__m128 D = VecShuffle_2323(inM.packed[2], inM.packed[3]);

	__m128 detSub = _mm_sub_ps(
		_mm_mul_ps(VecShuffle(inM.packed[0], inM.packed[2], 0,2,0,2), VecShuffle(inM.packed[1], inM.packed[3], 1,3,1,3)),
		_mm_mul_ps(VecShuffle(inM.packed[0], inM.packed[2], 1,3,1,3), VecShuffle(inM.packed[1], inM.packed[3], 0,2,0,2))
	);
	__m128 detA = VecSwizzle1(detSub, 0);
	__m128 detB = VecSwizzle1(detSub, 1);
	__m128 detC = VecSwizzle1(detSub, 2);
	__m128 detD = VecSwizzle1(detSub, 3);
	__m128 D_C = Mat2AdjMul(D, C);
	__m128 A_B = Mat2AdjMul(A, B);
	__m128 X_ = _mm_sub_ps(_mm_mul_ps(detD, A), Mat2Mul(B, D_C));
	__m128 W_ = _mm_sub_ps(_mm_mul_ps(detA, D), Mat2Mul(C, A_B));

	__m128 detM = _mm_mul_ps(detA, detD);
	__m128 Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), Mat2MulAdj(D, A_B));
	__m128 Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), Mat2MulAdj(A, D_C));
	detM = _mm_add_ps(detM, _mm_mul_ps(detB, detC));

	__m128 tr = _mm_mul_ps(A_B, VecSwizzle(D_C, 0,2,1,3));
	tr = _mm_hadd_ps(tr, tr);
	tr = _mm_hadd_ps(tr, tr);
	detM = _mm_sub_ps(detM, tr);

	const __m128 adjSignMask = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
	__m128 rDetM = _mm_div_ps(adjSignMask, detM);

	X_ = _mm_mul_ps(X_, rDetM);
	Y_ = _mm_mul_ps(Y_, rDetM);
	Z_ = _mm_mul_ps(Z_, rDetM);
	W_ = _mm_mul_ps(W_, rDetM);

	m4 r;
	r.packed[0] = VecShuffle(X_, Y_, 3,1,3,1);
	r.packed[1] = VecShuffle(X_, Y_, 2,0,2,0);
	r.packed[2] = VecShuffle(Z_, W_, 3,1,3,1);
	r.packed[3] = VecShuffle(Z_, W_, 2,0,2,0);
	return r;
}

// TODO(max): replace c lib random stuff
void rand_init(u32 seed) {
	srand(seed);
}

i32 randi() {
	return rand();
}

u32 randu() {
	return (u32)rand();
}

f32 randf() {
	return (f32)rand() / (f32)RAND_MAX;
}

f64 randf64() {
	return (f64)rand() / (f64)RAND_MAX;	
}

v3 rand_unit() {
	v3 ret = {randf(), randf(), randf()};
	return norm(ret);
}

#ifdef OSTREAM_OPS
std::ostream& operator<<(std::ostream& out, v2 r) {
	out << "{" << r.x << "," << r.y << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, uv2 r) {
	out << "{" << r.x << "," << r.y << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, v3 r) {
	out << "{" << r.x << "," << r.y << "," << r.z << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, iv3 r) {
	out << "{" << r.x << "," << r.y << "," << r.z << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, uv3 r) {
	out << "{" << r.x << "," << r.y << "," << r.z << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, v4 r) {
	out << "{" << r.x << "," << r.y << "," << r.z << "," << r.w << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, bv4 r) {
	out << "{" << r.x << "," << r.y << "," << r.z << "," << r.w << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, m4 r) {
	out << "{";
	for(i32 i = 0; i < 4; i++) {
		out << r.columns[i];
	}
	out << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, r2 r) {
	out << "{" << r.x << "," << r.y << "," << r.w << "," << r.h << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, ur2 r) {
	out << "{" << r.x << "," << r.y << "," << r.w << "," << r.h << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, ir2 r) {
	out << "{" << r.x << "," << r.y << "," << r.w << "," << r.h << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, color r) {
	out << "{" << r.r << "," << r.g << "," << r.b << "," << r.a << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, color3 r) {
	out << "{" << r.r << "," << r.g << "," << r.b << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, colorf r) {
	out << "{" << r.r << "," << r.g << "," << r.b << "," << r.a << "}";
	return out;
}
#endif
