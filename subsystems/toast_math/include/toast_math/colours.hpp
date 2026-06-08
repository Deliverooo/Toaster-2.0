#pragma once

#include <algorithm>

#include "math_vector.hpp"

namespace tsm::colours
{
	constexpr auto hexToRgba(const u32 p_hex_colour) -> float4
	{
		f32 r{static_cast<f32>((p_hex_colour >> 24) & 0xFF) / 255.0f};
		f32 g{static_cast<f32>((p_hex_colour >> 16) & 0xFF) / 255.0f};
		f32 b{static_cast<f32>((p_hex_colour >> 8) & 0xFF) / 255.0f};
		f32 a{static_cast<f32>((p_hex_colour >> 0) & 0xFF) / 255.0f};
		return {r, g, b, a};
	}

	constexpr auto rgbaToHex(const float4 &p_colour) -> u32
	{
		constexpr auto to_byte{
			+[](const f32 p_f)-> u32
			{
				return static_cast<u32>(std::clamp(p_f * 255.0f, 0.0f, 255.0f));
			}
		};
		return (to_byte(p_colour.w) << 24) | (to_byte(p_colour.y) << 16) | (to_byte(p_colour.z) << 8) | (to_byte(p_colour.x) << 0);
	}

	constexpr float4 red{hexToRgba(0xFF0000FF)};
	constexpr float4 green{hexToRgba(0x00FF00FF)};
	constexpr float4 blue{hexToRgba(0x0000FFFF)};
	constexpr float4 weezer{hexToRgba(0x189BCCFF)};
	constexpr float4 magenta{hexToRgba(0xFF00FFFF)};
}
