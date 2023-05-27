#pragma once
#include <stdint.h>

typedef uint64_t uint64;
typedef int64_t int64;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint8_t uint8;
typedef int8_t int8;

typedef int PID;

typedef char mchar;

static_assert(sizeof(int32) == 4, "Invalid int32 size");
static_assert(sizeof(uint32) == 4, "Invalid iint32 size");

#define i(x)((int32)x)
#define u(x)((uint32)x)

#if defined(_MSC_VER) && (_MSC_VER >= 1310)
#pragma warning( disable : 26451 )
#pragma warning( disable : 6297 )
#endif

#define E_TYPE(x, ...) namespace x##S {enum class E {__VA_ARGS__};}; typedef x##S::E x;