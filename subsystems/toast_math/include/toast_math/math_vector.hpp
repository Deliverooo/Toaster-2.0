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
