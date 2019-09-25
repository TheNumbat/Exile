
#pragma once

#include "string.h"
#include <stdio.h>

#define info(fmt, ...) (void)( \
    printf("[info] %s:%u: " fmt "\n", last_file(__FILE__).c_str, __LINE__, __VA_ARGS__), \
    fflush(stdout), 0) \

#define warn(fmt, ...) (void)( \
    printf("\033[0;31m[warn] %s:%u: " fmt "\033[0m\n", last_file(__FILE__).c_str, __LINE__, __VA_ARGS__), \
    fflush(stdout), 0)

#define die(fmt, ...) (void)( \
    printf("\033[1;31m[FATAL] %s:%u: " fmt "\033[0m\n", last_file(__FILE__).c_str, __LINE__, __VA_ARGS__), \
    fflush(stdout), \
    exit(__LINE__), 0)

#define fail_assert(fmt, file, line) ( \
    printf("\033[1;31m[ASSERT] %s:%u: " fmt "\033[0m\n", file, line), \
    fflush(stdout),\
    exit(__LINE__), 0)

#undef assert
#define assert(expr) (void)(                               \
        (!!(expr)) ||                                      \
        (fail_assert(#expr, last_file(__FILE__).c_str, __LINE__), 0))


