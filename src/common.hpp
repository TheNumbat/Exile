
#pragma once

#include <stdint.h>
#include "logger/log.hpp"

#ifdef ASSERTIONS
#include <assert.h>
#define INVALID_CODE_PATH assert(false);
#else
#define assert(a)
#define INVALID_CODE_PATH
#endif

typedef float		r32;
typedef double		r64;
typedef int8_t		s8;
typedef uint8_t		u8;
typedef int16_t		s16;
typedef uint16_t	u16;
typedef int32_t		s32;
typedef uint32_t	u32;
typedef int64_t		s64;
typedef uint64_t	u64;

#define file_path std::experimental::filesystem::v1::path