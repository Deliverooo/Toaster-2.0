#pragma once

#include "math_vector.hpp"
#include "toast_lib/system_types.h"

namespace tsm::colours
{
	constexpr auto hexToRgba(const uint32 p_hex_colour) -> float4
	{
		float32 r{static_cast<float32>((p_hex_colour >> 24) & 0xFF) / 255.0f};
		float32 g{static_cast<float32>((p_hex_colour >> 16) & 0xFF) / 255.0f};
		float32 b{static_cast<float32>((p_hex_colour >> 8) & 0xFF) / 255.0f};
		float32 a{static_cast<float32>((p_hex_colour >> 0) & 0xFF) / 255.0f};
		return {r, g, b, a};
	}

	constexpr auto rgbaToHex(const float4 &p_colour) -> uint32
	{
		constexpr auto to_byte{
			+[](const float32 p_f)-> uint32
			{
				return static_cast<uint32>(std::clamp(p_f * 255.0f, 0.0f, 255.0f));
			}
		};
		return (to_byte(p_colour.r) << 24) | (to_byte(p_colour.g) << 16) | (to_byte(p_colour.b) << 8) | (to_byte(p_colour.a) << 0);
	}

	constexpr float4 red{hexToRgba(0xFF0000FF)};
	constexpr float4 green{hexToRgba(0x00FF00FF)};
	constexpr float4 blue{hexToRgba(0x0000FFFF)};
	constexpr float4 weezer{hexToRgba(0x189BCCFF)};
	constexpr float4 magenta{hexToRgba(0xFF00FFFF)};
}
