#pragma once

#include "basic.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <intrin.h>
    #include <windows.h>
    #undef max
    #undef min
#elif defined(__linux__)
    #include <sys/mman.h>
#endif

// order matters!

#include "log.h"
#include "math.h"

#include "alloc_before.h"
#include "string_before.h"
#include "reflect.h"
#include "printf_before.h"

#include "vec.h"
#include "v_vec.h"
#include "array.h"
#include "stack.h"
#include "queue.h"
#include "heap.h"

#include "hash.h"
#include "map.h"

#include "alloc_after.h"
#include "string_after.h"
#include "printf_after.h"
