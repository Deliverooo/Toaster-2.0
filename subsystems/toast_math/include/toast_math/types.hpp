#pragma once

#include <cstdint>

namespace tsm
{
	using b32 = bool;

	using i8  = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;

	using u8  = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	using f32 = float;
	using f64 = double;

	using c8  = char8_t;
	using c16 = char16_t;
	using c32 = char32_t;
}

#include <cassert>
#define TSM_ASSERT_LENGTH(__length, __max) assert ((__length) >= 0 && (__length) < (__max))

#include <DirectXMath.h>
using namespace DirectX;