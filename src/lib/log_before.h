
#pragma once

inline void log(literal fmt, ...);

#define info(fmt, ...) (void)( \
    log("%s:%u [info] %s\n", last_file(literal(__FILE__)).c_str, __LINE__, \
    scratch_format(fmt, ##__VA_ARGS__).c_str), 0)

#define warn(fmt, ...) (void)( \
    log("\033[0;31m%s:%u [warn] %s\033[0m\n", last_file(literal(__FILE__)).c_str, __LINE__, \
    scratch_format(fmt, ##__VA_ARGS__).c_str), 0)

#define die(fmt, ...) (void)( \
    log("\033[1;31m%s:%u [FATAL] %s\033[0m\n", last_file(literal(__FILE__)).c_str, __LINE__, \
    scratch_format(fmt, ##__VA_ARGS__).c_str), exit(__LINE__), 0)

#ifdef _WIN32
#define fail_assert(msg, file, line) (void)( \
    log("\033[1;31m%s:%u [ASSERT] " msg "\033[0m\n", file, line), __debugbreak(), exit(__LINE__), 0)
#elif defined(__linux__)
#include <signal.h>
#define fail_assert(msg, file, line) (void)( \
    log("\033[1;31m%s:%u [ASSERT] " msg "\033[0m\n", file, line), raise(SIGTRAP), exit(__LINE__), 0)
#endif

#undef assert
#define assert(expr) (void)( \
        (!!(expr)) || \
        (fail_assert(#expr, last_file(literal(__FILE__)).c_str, __LINE__), 0))