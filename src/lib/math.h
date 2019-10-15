
#pragma once

#define _MAX(a,b) ((a) > (b) ? (a) : (b))
#define _MIN(a,b) ((a) < (b) ? (a) : (b))

#define KB(x) (1024 * (x))
#define MB(x) (1024 * KB(x))
#define GB(x) (1024 * MB(x))

inline u32 prev_pow2(u32 val) {

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

inline u32 ceil_pow2(u32 x) {
	u32 prev = prev_pow2(x);
	return x == prev ? x : prev << 1;
}
