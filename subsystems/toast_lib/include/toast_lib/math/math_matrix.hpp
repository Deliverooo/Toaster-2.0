#pragma once

#include <print>

#include "math_vector.hpp"

namespace tsm
{
	template<typename Type>
	struct mat2
	{
		using ColType = vec2<Type>;

		static constexpr uint32 s_dim{2u};

		ColType data[s_dim];

		constexpr mat2() = default;

		constexpr mat2(Type p_m00, Type p_m01, Type p_m10, Type p_m11) : data{ColType{p_m00, p_m10}, ColType{p_m01, p_m11}}
		{
		}

		constexpr mat2(ColType p_v1, ColType p_v2) : data{p_v1, p_v2}
		{
		}

		constexpr mat2(Type p_s) : data{{p_s, static_cast<Type>(0)}, {static_cast<Type>(0), p_s}}
		{
		}

		constexpr auto operator[](int32 p_index) -> ColType &
		{
			TSM_ASSERT_LENGTH(p_index, s_dim);
			return data[p_index];
		}

		constexpr auto operator[](int32 p_index) const -> const ColType &
		{
			TSM_ASSERT_LENGTH(p_index, s_dim);
			return data[p_index];
		}
	};

	template<typename Type>
	constexpr auto operator*(const mat2<Type> &p_m, Type p_s) -> mat2<Type> { return mat2{p_m[0] * p_s, p_m[1] * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const mat2<Type> &p_m) -> mat2<Type> { return mat2{p_s * p_m[0], p_s * p_m[1]}; }

	template<typename Type>
	constexpr auto operator*(const mat2<Type> &p_m, typename mat2<Type>::ColType p_v) -> mat2<Type>::ColType
	{
		return typename mat2<Type>::ColType{p_m[0][0] * p_v.x + p_m[1][0] * p_v.y, p_m[0][1] * p_v.x + p_m[1][1] * p_v.y};
	}

	template<typename Type>
	constexpr auto operator*(typename mat2<Type>::ColType p_v, const mat2<Type> &p_m) -> mat2<Type>::ColType
	{
		return typename mat2<Type>::ColType{p_v.x * p_m[0][0] + p_v.y * p_m[0][1], p_v.x * p_m[1][0] + p_v.y * p_m[1][1]};
	}

	template<typename Type>
	constexpr auto operator*(const mat2<Type> &p_m1, const mat2<Type> &p_m2) -> mat2<Type>
	{
		return mat2<Type>{
			p_m1[0][0] * p_m2[0][0] + p_m1[1][0] * p_m2[0][1],
			p_m1[0][1] * p_m2[0][0] + p_m1[1][1] * p_m2[0][1],
			p_m1[0][0] * p_m2[1][0] + p_m1[1][0] * p_m2[1][1],
			p_m1[0][1] * p_m2[1][0] + p_m1[1][1] * p_m2[1][1]
		};
	}

	template<typename Type>
	struct mat3
	{
		using ColType = vec3<Type>;

		static constexpr uint32 s_dim{3u};

		ColType data[s_dim];

		constexpr mat3() = default;

		constexpr mat3(Type p_m00, Type p_m01, Type p_m02, Type p_m10, Type p_m11, Type p_m12, Type p_m20, Type p_m21, Type p_m22) : data{
			{p_m00, p_m10, p_m20},
			{p_m01, p_m11, p_m21},
			{p_m02, p_m12, p_m22}
		}
		{
		}

		constexpr mat3(ColType p_v1, ColType p_v2, ColType p_v3) : data{p_v1, p_v2, p_v3}
		{
		}

		constexpr mat3(Type p_s) : data{
			{p_s, static_cast<Type>(0), static_cast<Type>(0)},
			{static_cast<Type>(0), p_s, static_cast<Type>(0)},
			{static_cast<Type>(0), static_cast<Type>(0), p_s}
		}
		{
		}

		constexpr auto operator[](int32 p_index) -> ColType &
		{
			TSM_ASSERT_LENGTH(p_index, s_dim);
			return data[p_index];
		}

		constexpr auto operator[](int32 p_index) const -> const ColType &
		{
			TSM_ASSERT_LENGTH(p_index, s_dim);
			return data[p_index];
		}
	};

	template<typename Type>
	constexpr auto operator*(const mat3<Type> &p_m, Type p_s) -> mat3<Type> { return mat3{p_m[0] * p_s, p_m[1] * p_s, p_m[2] * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const mat3<Type> &p_m) -> mat3<Type> { return mat3{p_s * p_m[0], p_s * p_m[1], p_s * p_m[2]}; }

	template<typename Type>
	constexpr auto operator*(const mat3<Type> &p_m, typename mat3<Type>::ColType p_v) -> mat3<Type>::ColType
	{
		return typename mat3<Type>::ColType{
			p_m[0][0] * p_v.x + p_m[1][0] * p_v.y + p_m[2][0] * p_v.z,
			p_m[0][1] * p_v.x + p_m[1][1] * p_v.y + p_m[2][1] * p_v.z,
			p_m[0][2] * p_v.x + p_m[1][2] * p_v.y + p_m[2][2] * p_v.z
		};
	}

	template<typename Type>
	constexpr auto operator*(typename mat3<Type>::ColType p_v, const mat3<Type> &p_m) -> mat3<Type>::ColType
	{
		return typename mat3<Type>::ColType{
			p_v.x * p_m[0][0] + p_v.y * p_m[1][0] + p_v.z * p_m[2][0],
			p_v.x * p_m[0][1] + p_v.y * p_m[1][1] + p_v.z * p_m[2][1],
			p_v.x * p_m[0][2] + p_v.y * p_m[1][2] + p_v.z * p_m[2][2]
		};
	}

	template<typename Type>
	constexpr auto operator*(const mat3<Type> &p_m1, const mat3<Type> &p_m2) -> mat3<Type>
	{
		return mat3<Type>{
			p_m1[0][0] * p_m2[0][0] + p_m1[1][0] * p_m2[0][1] + p_m1[2][0] * p_m2[0][2],
			p_m1[0][1] * p_m2[0][0] + p_m1[1][1] * p_m2[0][1] + p_m1[2][1] * p_m2[0][2],
			p_m1[0][2] * p_m2[0][0] + p_m1[1][2] * p_m2[0][1] + p_m1[2][2] * p_m2[0][2],

			// Column 1
			p_m1[0][0] * p_m2[1][0] + p_m1[1][0] * p_m2[1][1] + p_m1[2][0] * p_m2[1][2],
			p_m1[0][1] * p_m2[1][0] + p_m1[1][1] * p_m2[1][1] + p_m1[2][1] * p_m2[1][2],
			p_m1[0][2] * p_m2[1][0] + p_m1[1][2] * p_m2[1][1] + p_m1[2][2] * p_m2[1][2],

			// Column 2
			p_m1[0][0] * p_m2[2][0] + p_m1[1][0] * p_m2[2][1] + p_m1[2][0] * p_m2[2][2],
			p_m1[0][1] * p_m2[2][0] + p_m1[1][1] * p_m2[2][1] + p_m1[2][1] * p_m2[2][2],
			p_m1[0][2] * p_m2[2][0] + p_m1[1][2] * p_m2[2][1] + p_m1[2][2] * p_m2[2][2]
		};
	}

	template<typename Type>
	struct mat4
	{
		using ColType = vec4<Type>;

		static constexpr uint32 s_dim{4u};

		ColType data[s_dim];

		constexpr mat4() = default;

		constexpr mat4(Type p_m00, Type p_m01, Type p_m02, Type p_m03, Type p_m10, Type p_m11, Type p_m12, Type p_m13, Type p_m20, Type p_m21, Type p_m22, Type p_m23,
					   Type p_m30, Type p_m31, Type p_m32, Type p_m33) : data{
			{p_m00, p_m10, p_m20, p_m30},
			{p_m01, p_m11, p_m21, p_m31},
			{p_m02, p_m12, p_m22, p_m32},
			{p_m03, p_m13, p_m23, p_m33}
		}
		{
		}

		constexpr mat4(ColType p_v1, ColType p_v2, ColType p_v3, ColType p_v4) : data{p_v1, p_v2, p_v3, p_v4}
		{
		}

		constexpr mat4(Type p_s) : data{
			{p_s, static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0)},
			{static_cast<Type>(0), p_s, static_cast<Type>(0), static_cast<Type>(0)},
			{static_cast<Type>(0), static_cast<Type>(0), p_s, static_cast<Type>(0)},
			{static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0), p_s}
		}
		{
		}

		constexpr mat4(const mat3<Type> &p_m) : data{
			{p_m[0], static_cast<Type>(0)},
			{p_m[1], static_cast<Type>(0)},
			{p_m[2], static_cast<Type>(0)},
			{static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(1)}
		}
		{
		}

		constexpr auto operator[](int32 p_index) -> ColType &
		{
			TSM_ASSERT_LENGTH(p_index, s_dim);
			return data[p_index];
		}

		constexpr auto operator[](int32 p_index) const -> const ColType &
		{
			TSM_ASSERT_LENGTH(p_index, s_dim);
			return data[p_index];
		}
	};

	template<typename Type>
	constexpr auto operator*(const mat4<Type> &p_m, Type p_s) -> mat4<Type> { return mat4{p_m[0] * p_s, p_m[1] * p_s, p_m[2] * p_s, p_m[3] * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const mat4<Type> &p_m) -> mat4<Type> { return mat4{p_s * p_m[0], p_s * p_m[1], p_s * p_m[2], p_s * p_m[3]}; }

	template<typename Type>
	constexpr auto operator*(const mat4<Type> &p_m, typename mat4<Type>::ColType p_v) -> mat4<Type>::ColType
	{
		return typename mat4<Type>::ColType{
			p_m[0][0] * p_v.x + p_m[1][0] * p_v.y + p_m[2][0] * p_v.z + p_m[3][0] * p_v.w,
			p_m[0][1] * p_v.x + p_m[1][1] * p_v.y + p_m[2][1] * p_v.z + p_m[3][1] * p_v.w,
			p_m[0][2] * p_v.x + p_m[1][2] * p_v.y + p_m[2][2] * p_v.z + p_m[3][2] * p_v.w,
			p_m[0][3] * p_v.x + p_m[1][3] * p_v.y + p_m[2][3] * p_v.z + p_m[3][3] * p_v.w
		};
	}

	template<typename Type>
	constexpr auto operator*(typename mat4<Type>::ColType p_v, const mat4<Type> &p_m) -> mat4<Type>::ColType
	{
		return typename mat4<Type>::ColType{
			p_v.x * p_m[0][0] + p_v.y * p_m[1][0] + p_v.z * p_m[2][0] + p_v.w * p_m[3][0],
			p_v.x * p_m[0][1] + p_v.y * p_m[1][1] + p_v.z * p_m[2][1] + p_v.w * p_m[3][1],
			p_v.x * p_m[0][2] + p_v.y * p_m[1][2] + p_v.z * p_m[2][2] + p_v.w * p_m[3][2],
			p_v.x * p_m[0][3] + p_v.y * p_m[1][3] + p_v.z * p_m[2][3] + p_v.w * p_m[3][3]
		};
	}

	template<typename Type>
	constexpr auto operator*(const mat4<Type> &p_m1, const mat4<Type> &p_m2) -> mat4<Type>
	{
		return mat4<Type>{
			// Column 0
			p_m1[0][0] * p_m2[0][0] + p_m1[1][0] * p_m2[0][1] + p_m1[2][0] * p_m2[0][2] + p_m1[3][0] * p_m2[0][3],
			p_m1[0][1] * p_m2[0][0] + p_m1[1][1] * p_m2[0][1] + p_m1[2][1] * p_m2[0][2] + p_m1[3][1] * p_m2[0][3],
			p_m1[0][2] * p_m2[0][0] + p_m1[1][2] * p_m2[0][1] + p_m1[2][2] * p_m2[0][2] + p_m1[3][2] * p_m2[0][3],
			p_m1[0][3] * p_m2[0][0] + p_m1[1][3] * p_m2[0][1] + p_m1[2][3] * p_m2[0][2] + p_m1[3][3] * p_m2[0][3],

			// Column 1
			p_m1[0][0] * p_m2[1][0] + p_m1[1][0] * p_m2[1][1] + p_m1[2][0] * p_m2[1][2] + p_m1[3][0] * p_m2[1][3],
			p_m1[0][1] * p_m2[1][0] + p_m1[1][1] * p_m2[1][1] + p_m1[2][1] * p_m2[1][2] + p_m1[3][1] * p_m2[1][3],
			p_m1[0][2] * p_m2[1][0] + p_m1[1][2] * p_m2[1][1] + p_m1[2][2] * p_m2[1][2] + p_m1[3][2] * p_m2[1][3],
			p_m1[0][3] * p_m2[1][0] + p_m1[1][3] * p_m2[1][1] + p_m1[2][3] * p_m2[1][2] + p_m1[3][3] * p_m2[1][3],

			// Column 2
			p_m1[0][0] * p_m2[2][0] + p_m1[1][0] * p_m2[2][1] + p_m1[2][0] * p_m2[2][2] + p_m1[3][0] * p_m2[2][3],
			p_m1[0][1] * p_m2[2][0] + p_m1[1][1] * p_m2[2][1] + p_m1[2][1] * p_m2[2][2] + p_m1[3][1] * p_m2[2][3],
			p_m1[0][2] * p_m2[2][0] + p_m1[1][2] * p_m2[2][1] + p_m1[2][2] * p_m2[2][2] + p_m1[3][2] * p_m2[2][3],
			p_m1[0][3] * p_m2[2][0] + p_m1[1][3] * p_m2[2][1] + p_m1[2][3] * p_m2[2][2] + p_m1[3][3] * p_m2[2][3],

			// Column 3
			p_m1[0][0] * p_m2[3][0] + p_m1[1][0] * p_m2[3][1] + p_m1[2][0] * p_m2[3][2] + p_m1[3][0] * p_m2[3][3],
			p_m1[0][1] * p_m2[3][0] + p_m1[1][1] * p_m2[3][1] + p_m1[2][1] * p_m2[3][2] + p_m1[3][1] * p_m2[3][3],
			p_m1[0][2] * p_m2[3][0] + p_m1[1][2] * p_m2[3][1] + p_m1[2][2] * p_m2[3][2] + p_m1[3][2] * p_m2[3][3],
			p_m1[0][3] * p_m2[3][0] + p_m1[1][3] * p_m2[3][1] + p_m1[2][3] * p_m2[3][2] + p_m1[3][3] * p_m2[3][3]
		};
	}

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

	constexpr auto scale(const float4x4 &p_m, const float3 &p_v) -> float4x4
	{
		float4x4 result{};
		result[0] = p_m[0] * p_v[0];
		result[1] = p_m[1] * p_v[1];
		result[2] = p_m[2] * p_v[2];
		result[3] = p_m[3];
		return result;
	}

	template<typename Type>
	constexpr auto deternimant(const mat2<Type> &p_m) -> Type
	{
		return {p_m[0][0] * p_m[1][1] - p_m[1][0] * p_m[0][1]};
	}

	template<typename Type>
	constexpr auto deternimant(const mat3<Type> &p_m) -> Type
	{
		Type d1{p_m[0][0] * (p_m[1][1] * p_m[2][2] - p_m[2][1] * p_m[1][2])};
		Type d2{p_m[1][0] * (p_m[0][1] * p_m[2][2] - p_m[2][1] * p_m[0][2])};
		Type d3{p_m[2][0] * (p_m[0][1] * p_m[1][2] - p_m[1][1] * p_m[0][2])};
		return {d1 - d2 + d3};
	}

	template<typename Type>
	constexpr auto deternimant(const mat4<Type> &p_m) -> Type
	{
		const Type m00{p_m[0][0]};
		const Type m01{p_m[0][1]};
		const Type m02{p_m[0][2]};
		const Type m03{p_m[0][3]};
		const Type m10{p_m[1][0]};
		const Type m11{p_m[1][1]};
		const Type m12{p_m[1][2]};
		const Type m13{p_m[1][3]};
		const Type m20{p_m[2][0]};
		const Type m21{p_m[2][1]};
		const Type m22{p_m[2][2]};
		const Type m23{p_m[2][3]};
		const Type m30{p_m[3][0]};
		const Type m31{p_m[3][1]};
		const Type m32{p_m[3][2]};
		const Type m33{p_m[3][3]};

		// Compute minors for the first column to calculate the determinant later
		const Type sub0{m22 * m33 - m32 * m23};
		const Type sub1{m21 * m33 - m31 * m23};
		const Type sub2{m21 * m32 - m31 * m22};
		const Type sub3{m20 * m33 - m30 * m23};
		const Type sub4{m20 * m32 - m30 * m22};
		const Type sub5{m20 * m31 - m30 * m21};

		// Calculate the adjugate matrix elements directly into columns
		typename mat4<Type>::ColType c0{
			(m11 * sub0 - m12 * sub1 + m13 * sub2),
			-(m01 * sub0 - m02 * sub1 + m03 * sub2),
			(m01 * (m12 * m33 - m32 * m13) - m02 * (m11 * m33 - m31 * m13) + m03 * (m11 * m32 - m31 * m12)),
			-(m01 * (m12 * m23 - m22 * m13) - m02 * (m11 * m23 - m21 * m13) + m03 * (m11 * m22 - m21 * m12))
		};

		// Calculate determinant using the first column of the adjugate matrix
		return m00 * c0[0] + m10 * c0[1] + m20 * c0[2] + m30 * c0[3];
	}

	template<typename Type>
	constexpr auto inverse(const mat2<Type> &p_m) -> mat2<Type>
	{
		Type one_over_deternimant{static_cast<Type>(1) / deternimant(p_m)};
		mat2 Inverse{p_m[1][1] * one_over_deternimant, -p_m[0][1] * one_over_deternimant, -p_m[1][0] * one_over_deternimant, p_m[0][0] * one_over_deternimant};
		return Inverse;
	}

	template<typename Type>
	constexpr auto inverse(const mat3<Type> &p_m) -> mat3<Type>
	{
		Type one_over_deternimant{
			static_cast<Type>(1) / (+p_m[0][0] * (p_m[1][1] * p_m[2][2] - p_m[2][1] * p_m[1][2]) - p_m[1][0] * (p_m[0][1] * p_m[2][2] - p_m[2][1] * p_m[0][2]) + p_m[2][0]
									* (p_m[0][1] * p_m[1][2] - p_m[1][1] * p_m[0][2]))
		};

		mat3<Type> inverse;
		inverse[0][0] = +(p_m[1][1] * p_m[2][2] - p_m[2][1] * p_m[1][2]);
		inverse[1][0] = -(p_m[1][0] * p_m[2][2] - p_m[2][0] * p_m[1][2]);
		inverse[2][0] = +(p_m[1][0] * p_m[2][1] - p_m[2][0] * p_m[1][1]);
		inverse[0][1] = -(p_m[0][1] * p_m[2][2] - p_m[2][1] * p_m[0][2]);
		inverse[1][1] = +(p_m[0][0] * p_m[2][2] - p_m[2][0] * p_m[0][2]);
		inverse[2][1] = -(p_m[0][0] * p_m[2][1] - p_m[2][0] * p_m[0][1]);
		inverse[0][2] = +(p_m[0][1] * p_m[1][2] - p_m[1][1] * p_m[0][2]);
		inverse[1][2] = -(p_m[0][0] * p_m[1][2] - p_m[1][0] * p_m[0][2]);
		inverse[2][2] = +(p_m[0][0] * p_m[1][1] - p_m[1][0] * p_m[0][1]);

		inverse = inverse * one_over_deternimant;
		return inverse;
	}

	template<typename Type>
	constexpr auto inverse(const mat4<Type> &p_m) -> mat4<Type>
	{
		const Type m00{p_m[0][0]};
		const Type m01{p_m[0][1]};
		const Type m02{p_m[0][2]};
		const Type m03{p_m[0][3]};
		const Type m10{p_m[1][0]};
		const Type m11{p_m[1][1]};
		const Type m12{p_m[1][2]};
		const Type m13{p_m[1][3]};
		const Type m20{p_m[2][0]};
		const Type m21{p_m[2][1]};
		const Type m22{p_m[2][2]};
		const Type m23{p_m[2][3]};
		const Type m30{p_m[3][0]};
		const Type m31{p_m[3][1]};
		const Type m32{p_m[3][2]};
		const Type m33{p_m[3][3]};

		// Compute minors for the first column to calculate the determinant later
		const Type sub0{m22 * m33 - m32 * m23};
		const Type sub1{m21 * m33 - m31 * m23};
		const Type sub2{m21 * m32 - m31 * m22};
		const Type sub3{m20 * m33 - m30 * m23};
		const Type sub4{m20 * m32 - m30 * m22};
		const Type sub5{m20 * m31 - m30 * m21};

		// Calculate the adjugate matrix elements directly into columns
		typename mat4<Type>::ColType c0{
			(m11 * sub0 - m12 * sub1 + m13 * sub2),
			-(m01 * sub0 - m02 * sub1 + m03 * sub2),
			(m01 * (m12 * m33 - m32 * m13) - m02 * (m11 * m33 - m31 * m13) + m03 * (m11 * m32 - m31 * m12)),
			-(m01 * (m12 * m23 - m22 * m13) - m02 * (m11 * m23 - m21 * m13) + m03 * (m11 * m22 - m21 * m12))
		};

		// Calculate determinant using the first column of the adjugate matrix
		const Type det{m00 * c0[0] + m10 * c0[1] + m20 * c0[2] + m30 * c0[3]};

		// Check if matrix is singular (non-invertible)
		if (std::abs(det) <= static_cast<Type>(0))
		{
			TST_ASSERT(false);
			return mat4<Type>{};
		}

		const Type one_over_determinant{static_cast<Type>(1) / det};

		// Complete the remaining columns of the adjugate matrix
		typename mat4<Type>::ColType c1{
			-(m10 * sub0 - m12 * sub3 + m13 * sub4),
			(m00 * sub0 - m02 * sub3 + m03 * sub4),
			-(m00 * (m12 * m33 - m32 * m13) - m02 * (m10 * m33 - m30 * m13) + m03 * (m10 * m32 - m30 * m12)),
			(m00 * (m12 * m23 - m22 * m13) - m02 * (m10 * m23 - m30 * m13) + m03 * (m10 * m22 - m30 * m12))
		};

		typename mat4<Type>::ColType c2{
			(m10 * sub1 - m11 * sub3 + m13 * sub5),
			-(m00 * sub1 - m01 * sub3 + m03 * sub5),
			(m00 * (m11 * m33 - m31 * m13) - m01 * (m10 * m33 - m30 * m13) + m03 * (m10 * m31 - m30 * m11)),
			-(m00 * (m11 * m23 - m21 * m13) - m01 * (m10 * m23 - m30 * m13) + m03 * (m10 * m21 - m30 * m11))
		};

		typename mat4<Type>::ColType c3{
			-(m10 * sub2 - m11 * sub4 + m12 * sub5),
			(m00 * sub2 - m01 * sub4 + m02 * sub5),
			-(m00 * (m11 * m32 - m31 * m12) - m01 * (m10 * m32 - m30 * m12) + m02 * (m10 * m31 - m30 * m11)),
			(m00 * (m11 * m22 - m21 * m12) - m01 * (m10 * m22 - m30 * m12) + m02 * (m10 * m21 - m30 * m11))
		};

		// Construct the result by multiplying the adjugate columns by 1/det
		return mat4<Type>{c0 * one_over_determinant, c1 * one_over_determinant, c2 * one_over_determinant, c3 * one_over_determinant};
	}

	template<typename Type>
	constexpr auto transpose(const mat2<Type> &p_m) -> mat2<Type>
	{
		mat2<Type> result{static_cast<Type>(1)};
		result[0][0] = p_m[0][0];
		result[0][1] = p_m[1][0];
		result[1][0] = p_m[0][1];
		result[1][1] = p_m[1][1];
		return result;
	}

	template<typename Type>
	constexpr auto transpose(const mat3<Type> &p_m) -> mat3<Type>
	{
		mat3<Type> result{static_cast<Type>(1)};
		result[0][0] = p_m[0][0];
		result[0][1] = p_m[1][0];
		result[0][2] = p_m[2][0];

		result[1][0] = p_m[0][1];
		result[1][1] = p_m[1][1];
		result[1][2] = p_m[2][1];

		result[2][0] = p_m[0][2];
		result[2][1] = p_m[1][2];
		result[2][2] = p_m[2][2];
		return result;
	}

	template<typename Type>
	constexpr auto transpose(const mat4<Type> &p_m) -> mat4<Type>
	{
		mat4<Type> result{static_cast<Type>(1)};
		result[0][0] = p_m[0][0];
		result[0][1] = p_m[1][0];
		result[0][2] = p_m[2][0];
		result[0][3] = p_m[3][0];

		result[1][0] = p_m[0][1];
		result[1][1] = p_m[1][1];
		result[1][2] = p_m[2][1];
		result[1][3] = p_m[3][1];

		result[2][0] = p_m[0][2];
		result[2][1] = p_m[1][2];
		result[2][2] = p_m[2][2];
		result[2][3] = p_m[3][2];

		result[3][0] = p_m[0][3];
		result[3][1] = p_m[1][3];
		result[3][2] = p_m[2][3];
		result[3][3] = p_m[3][3];
		return result;
	}

	template<typename Type>
	constexpr auto perspective(Type p_fov, Type p_aspect, Type p_near, Type p_far) -> mat4<Type>
	{
		const Type tan_half_fovy{std::tanf(p_fov / static_cast<Type>(2))};

		mat4<Type> result(static_cast<Type>(0));
		result[0][0] = static_cast<Type>(1) / (p_aspect * tan_half_fovy);
		result[1][1] = static_cast<Type>(1) / (tan_half_fovy);
		result[2][2] = -(p_far + p_near) / (p_far - p_near);
		result[2][3] = -static_cast<Type>(1);
		result[3][2] = -(static_cast<Type>(2) * p_far * p_near) / (p_far - p_near);
		return result;
	}

	template<typename Type>
	constexpr auto ortho(Type p_left, Type p_right, Type p_bottom, Type p_top) -> mat4<Type>
	{
		mat4<Type> result{static_cast<Type>(1)};
		result[0][0] = static_cast<Type>(2) / (p_right - p_left);
		result[1][1] = static_cast<Type>(2) / (p_top - p_bottom);
		result[2][2] = -static_cast<Type>(1);
		result[3][0] = -(p_right + p_left) / (p_right - p_left);
		result[3][1] = -(p_top + p_bottom) / (p_top - p_bottom);
		return result;
	}

	template<typename Type>
	constexpr auto ortho(Type p_left, Type p_right, Type p_bottom, Type p_top, Type p_near, Type p_far) -> mat4<Type>
	{
		mat4<Type> result{1};
		result[0][0] = static_cast<Type>(2) / (p_right - p_left);
		result[1][1] = static_cast<Type>(2) / (p_top - p_bottom);
		result[2][2] = -static_cast<Type>(2) / (p_far - p_near);
		result[3][0] = -(p_right + p_left) / (p_right - p_left);
		result[3][1] = -(p_top + p_bottom) / (p_top - p_bottom);
		result[3][2] = -(p_far + p_near) / (p_far - p_near);
		return result;
	}
}
