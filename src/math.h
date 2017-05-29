
#pragma once

// TODO(max): remove this, add intrinsics
#include <cmath>

// TODO(max): replace with faster operation
inline u32 mod(u32 dividend, u32 devisor) {
	return dividend % devisor;
}

#define abs(v) _abs(v)
inline i32 _abs(i32 value) {
	return value > 0 ? value : -value;
}
#define fabs(v) _fabs(v)
inline f32 _fabs(f32 value) {
	return value > 0.0f ? value : -value;	
}
inline f64 _fabs(f64 value) {
	return value > 0.0f ? value : -value;	
}

inline f32 _roundf(f32 value) {
	return roundf(value);
}
#define roundf(v) _roundf(v)

inline f32 _ceilf(f32 value) {
	return ceilf(value);
}
#define ceilf(v) _ceilf(v)
