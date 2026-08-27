#pragma once

#include "mat4.hpp"

namespace tsm
{
	using bool2x2 = Mat2<b32>;
	using bool3x3 = Mat3<b32>;
	using bool4x4 = Mat4<b32>;

	using int2x2 = Mat2<i32>;
	using int3x3 = Mat3<i32>;
	using int4x4 = Mat4<i32>;

	using uint2x2 = Mat2<u32>;
	using uint3x3 = Mat3<u32>;
	using uint4x4 = Mat4<u32>;

	using float2x2 = Mat2<f32>;
	using float3x3 = Mat3<f32>;
	using float4x4 = Mat4<f32>;

	using double2x2 = Mat2<f64>;
	using double3x3 = Mat3<f64>;
	using double4x4 = Mat4<f64>;

	template<typename Type>
	constexpr auto translate(const Mat4<Type> &p_m, const Vec3<Type> &p_v) -> Mat4<Type>
	{
		Mat4<Type> result{p_m};
		result[3].x = result[3].x + p_v.x;
		result[3].y = result[3].y + p_v.y;
		result[3].z = result[3].z + p_v.z;
		return result;
	}

	template<typename Type>
	constexpr auto scale(const Mat4<Type> &p_m, const Vec3<Type> &p_v) -> Mat4<Type>
	{
		Mat4<Type> result{};
		result[0] = p_m[0] * p_v[0];
		result[1] = p_m[1] * p_v[1];
		result[2] = p_m[2] * p_v[2];
		result[3] = p_m[3];
		return result;
	}

	template<typename Type>
	constexpr auto perspective(Type p_fov, Type p_aspect, Type p_near, Type p_far) -> Mat4<Type>
	{
		const Type tan_half_fovy{std::tanf(p_fov / static_cast<Type>(2))};

		Mat4<Type> result(static_cast<Type>(0));
		result[0][0] = static_cast<Type>(1) / (p_aspect * tan_half_fovy);
		result[1][1] = static_cast<Type>(1) / (tan_half_fovy);
		result[2][2] = -(p_far + p_near) / (p_far - p_near);
		result[2][3] = -static_cast<Type>(1);
		result[3][2] = -(static_cast<Type>(2) * p_far * p_near) / (p_far - p_near);
		return result;
	}

	template<typename Type>
	constexpr auto ortho(Type p_left, Type p_right, Type p_bottom, Type p_top) -> Mat4<Type>
	{
		Mat4<Type> result{static_cast<Type>(1)};
		result[0][0] = static_cast<Type>(2) / (p_right - p_left);
		result[1][1] = static_cast<Type>(2) / (p_top - p_bottom);
		result[2][2] = -static_cast<Type>(1);
		result[3][0] = -(p_right + p_left) / (p_right - p_left);
		result[3][1] = -(p_top + p_bottom) / (p_top - p_bottom);
		return result;
	}

	template<typename Type>
	constexpr auto ortho(Type p_left, Type p_right, Type p_bottom, Type p_top, Type p_near, Type p_far) -> Mat4<Type>
	{
		Mat4<Type> result{1};
		result[0][0] = static_cast<Type>(2) / (p_right - p_left);
		result[1][1] = static_cast<Type>(2) / (p_top - p_bottom);
		result[2][2] = -static_cast<Type>(2) / (p_far - p_near);
		result[3][0] = -(p_right + p_left) / (p_right - p_left);
		result[3][1] = -(p_top + p_bottom) / (p_top - p_bottom);
		result[3][2] = -(p_far + p_near) / (p_far - p_near);
		return result;
	}
}
