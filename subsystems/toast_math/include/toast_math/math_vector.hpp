#pragma once

#include "math_constants.hpp"
#include "vec4.hpp"

namespace tsm
{
	using bool2 = Vec2<b32>;
	using bool3 = Vec3<b32>;
	using bool4 = Vec4<b32>;

	using int2 = Vec2<i32>;
	using int3 = Vec3<i32>;
	using int4 = Vec4<i32>;

	using uint2 = Vec2<u32>;
	using uint3 = Vec3<u32>;
	using uint4 = Vec4<u32>;

	using float2 = Vec2<f32>;
	using float3 = Vec3<f32>;
	using float4 = Vec4<f32>;

	using double2 = Vec2<f64>;
	using double3 = Vec3<f64>;
	using double4 = Vec4<f64>;

	using Extent2D = uint2;

	struct Rect
	{
		constexpr Rect() = default;

		constexpr Rect(uint2 p_size) : size(p_size), offset(0u)
		{
		}

		uint2 size{0u};
		int2  offset{0u};
	};

	struct Viewport // Should map directly to vk::Viewport
	{
		constexpr Viewport() = default;

		constexpr Viewport(float2 p_size) : offset(float2::zero), size(p_size), depthBounds(0.0f, 1.0f)
		{
		}

		float2 offset{float2::zero};
		float2 size{float2::zero};
		float2 depthBounds{0.0f, 1.0f}; // Min depth and max depth
	};

	const XMVECTORI32 vulkanUpDir{.v{0.0f, -1.0f, 0.0f, 1.0f}};
}

// Incredibly useful for cameras where you want to normalize the change in position without it being infinity... :)
namespace DirectX
{
	inline XMVECTOR XM_CALLCONV XMVector2NormalizeSafe(FXMVECTOR p_vector)
	{
		XMVECTOR length{XMVector2LengthSq(p_vector)};
		XMVECTOR control{XMVectorGreater(length, XMVectorSplatEpsilon())};
		XMVECTOR normalized{XMVector2Normalize(p_vector)};
		return XMVectorSelect(XMVectorZero(), normalized, control);
	}

	inline XMVECTOR XM_CALLCONV XMVector3NormalizeSafe(FXMVECTOR p_vector)
	{
		XMVECTOR length{XMVector3LengthSq(p_vector)};
		XMVECTOR control{XMVectorGreater(length, XMVectorSplatEpsilon())};
		XMVECTOR normalized{XMVector3Normalize(p_vector)};
		return XMVectorSelect(XMVectorZero(), normalized, control);
	}

	inline XMVECTOR XM_CALLCONV XMVector4NormalizeSafe(FXMVECTOR p_vector)
	{
		XMVECTOR length{XMVector4LengthSq(p_vector)};
		XMVECTOR control{XMVectorGreater(length, XMVectorSplatEpsilon())};
		XMVECTOR normalized{XMVector4Normalize(p_vector)};
		return XMVectorSelect(XMVectorZero(), normalized, control);
	}
}
