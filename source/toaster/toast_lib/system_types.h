#pragma once

#if defined(__cplusplus)
#include <cstddef> // (C++) std::nullptr_t
#include <cstdint> // _t typedefs

using int8  = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using float32 = float;
using float64 = double;

#else
#include <stddef.h> // (C++) std::nullptr_t
#include <stdint.h> // _t typedefs

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float float32;
typedef double float64;
#endif
