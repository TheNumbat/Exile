
#pragma once

#include "string.h"
#include <stdio.h>

#define info(fmt, ...) (void)( \
    printf("%s:%u [info] %s\n", last_file(__FILE__).c_str, __LINE__, scratch_format(string::literal(fmt), ##__VA_ARGS__).c_str), \
    fflush(stdout), 0) \

#define warn(fmt, ...) (void)( \
    printf("\033[0;31m%s:%u [warn] %s\033[0m\n", last_file(__FILE__).c_str, __LINE__, scratch_format(string::literal(fmt), ##__VA_ARGS__).c_str), \
    fflush(stdout), 0)

#define die(fmt, ...) (void)( \
    printf("\033[1;31m%s:%u [FATAL] %s\033[0m\n", last_file(__FILE__).c_str, __LINE__, scratch_format(string::literal(fmt), ##__VA_ARGS__).c_str), \
    fflush(stdout), \
    exit(__LINE__), 0)

#define fail_assert(msg, file, line) ( \
    printf("\033[1;31m%s:%u [ASSERT] " msg "\033[0m\n", file, line), \
    fflush(stdout),\
    exit(__LINE__), 0)

#undef assert
#define assert(expr) (void)(                               \
        (!!(expr)) ||                                      \
        (fail_assert(#expr, last_file(__FILE__).c_str, __LINE__), 0))

#include "printf.h"

