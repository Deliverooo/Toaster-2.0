#pragma once

#include "math_vector.hpp"

namespace tsm
{
	template<typename Type>
	struct mat2
	{
		using ColType = vec2<Type>;

		static constexpr uint32 s_dim{2u};

		ColType data[s_dim];

		constexpr mat2() : data(static_cast<Type>(0), static_cast<Type>(0))
		{
		}

		constexpr mat2(Type p_m00, Type p_m01, Type p_m10, Type p_m11) : data({p_m00, p_m10}, {p_m01, p_m11})
		{
		}

		constexpr mat2(ColType p_v1, ColType p_v2) : data(p_v1, p_v2)
		{
		}

		constexpr mat2(Type p_s) : data({p_s, static_cast<Type>(0)}, {static_cast<Type>(0), p_s})
		{
		}

		constexpr auto operator[](int32 p_index) -> ColType &
		{
			return data[p_index];
		}

		constexpr auto operator[](int32 p_index) const -> const ColType &
		{
			return data[p_index];
		}
	};

	template<typename Type>
	struct mat3
	{
		using ColType = vec3<Type>;

		static constexpr uint32 s_dim{3u};

		ColType data[s_dim];

		constexpr mat3() : data(static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0))
		{
		}

		constexpr mat3(Type p_m00, Type p_m01, Type p_m02, Type p_m10, Type p_m11, Type p_m12, Type p_m20, Type p_m21, Type p_m22) : data({p_m00, p_m10, p_m20},
																																		  {p_m01, p_m11, p_m21}, {
																																			  p_m02,
																																			  p_m12,
																																			  p_m22
																																		  })
		{
		}

		constexpr mat3(ColType p_v1, ColType p_v2, ColType p_v3) : data(p_v1, p_v2, p_v3)
		{
		}

		constexpr mat3(Type p_s) : data({p_s, static_cast<Type>(0), static_cast<Type>(0)}, {static_cast<Type>(0), p_s, static_cast<Type>(0)},
										{static_cast<Type>(0), static_cast<Type>(0), p_s})
		{
		}

		constexpr auto operator[](int32 p_index) -> ColType &
		{
			return data[p_index];
		}

		constexpr auto operator[](int32 p_index) const -> const ColType &
		{
			return data[p_index];
		}
	};

	template<typename Type>
	struct mat4
	{
		using ColType = vec4<Type>;

		static constexpr uint32 s_dim{4u};

		ColType data[s_dim];

		constexpr mat4() : data(static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0))
		{
		}

		constexpr mat4(Type p_m00, Type p_m01, Type p_m02, Type p_m03, Type p_m10, Type p_m11, Type p_m12, Type p_m13, Type p_m20, Type p_m21, Type p_m22, Type p_m23,
					   Type p_m30, Type p_m31, Type p_m32, Type p_m33) : data({p_m00, p_m10, p_m20, p_m30}, {p_m01, p_m11, p_m21, p_m31}, {p_m02, p_m12, p_m22, p_m32},
																			  {p_m03, p_m13, p_m23, p_m33})
		{
		}

		constexpr mat4(ColType p_v1, ColType p_v2, ColType p_v3, ColType p_v4) : data(p_v1, p_v2, p_v3, p_v4)
		{
		}

		constexpr mat4(Type p_s) : data({p_s, static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0)},
										{static_cast<Type>(0), p_s, static_cast<Type>(0), static_cast<Type>(0)},
										{static_cast<Type>(0), static_cast<Type>(0), p_s, static_cast<Type>(0)}, {
											static_cast<Type>(0),
											static_cast<Type>(0),
											static_cast<Type>(0),
											p_s
										})
		{
		}

		constexpr auto operator[](int32 p_index) -> ColType &
		{
			return data[p_index];
		}

		constexpr auto operator[](int32 p_index) const -> const ColType &
		{
			return data[p_index];
		}
	};

	using bool1x1 = bool32;
	using bool2x2 = mat2<bool32>;
	using bool3x3 = mat3<bool32>;
	using bool4x4 = mat4<bool32>;

	using int1x1 = int32;
	using int2x2 = mat2<int32>;
	using int3x3 = mat3<int32>;
	using int4x4 = mat4<int32>;

	using uint1x1 = uint32;
	using uint2x2 = mat2<uint32>;
	using uint3x3 = mat3<uint32>;
	using uint4x4 = mat4<uint32>;

	using float1x1 = float32;
	using float2x2 = mat2<float32>;
	using float3x3 = mat3<float32>;
	using float4x4 = mat4<float32>;

	using double1x1 = float64;
	using double2x2 = mat2<float64>;
	using double3x3 = mat3<float64>;
	using double4x4 = mat4<float64>;

	constexpr auto translate(const float4x4 &p_m, const float3 &p_v) -> float4x4
	{
		float4x4 result{p_m};
		result[3] = p_m[0] * p_v[0] + p_m[1] * p_v[1] + p_m[2] * p_v[2] + p_m[3];
		return result;
	}

	// constexpr auto decomposeTransform(const float4x4 &p_transform, float3 &p_out_translation, quat &p_out_orientation, float3 &p_out_scale) -> void
	// {
	// 	p_out_translation = float3(p_transform[3]);
	//
	// 	p_out_scale.x = glm::length(float3(p_transform[0]));
	// 	p_out_scale.y = glm::length(float3(p_transform[1]));
	// 	p_out_scale.z = glm::length(float3(p_transform[2]));
	//
	// 	const float3x3 rot_mat = {float3(p_transform[0]) / p_out_scale.x, p_transform[1] / p_out_scale.y, p_transform[2] / p_out_scale.z};
	//
	// 	p_out_orientation = glm::quat_cast(rot_mat);
	// }
}
