#pragma once

#include "mat2.hpp"

namespace tsm
{
	template<typename Type>
	struct Mat3
	{
		using ColType = Vec3<Type>;

		static constexpr u32 s_dim{3u};

		ColType data[s_dim];

		constexpr Mat3() = default;

		constexpr Mat3(Type p_m00, Type p_m01, Type p_m02, Type p_m10, Type p_m11, Type p_m12, Type p_m20, Type p_m21, Type p_m22) : data{
			{p_m00, p_m10, p_m20},
			{p_m01, p_m11, p_m21},
			{p_m02, p_m12, p_m22}
		}
		{
		}

		constexpr Mat3(ColType p_v1, ColType p_v2, ColType p_v3) : data{p_v1, p_v2, p_v3}
		{
		}

		constexpr Mat3(Type p_s) : data{
			{p_s, static_cast<Type>(0), static_cast<Type>(0)},
			{static_cast<Type>(0), p_s, static_cast<Type>(0)},
			{static_cast<Type>(0), static_cast<Type>(0), p_s}
		}
		{
		}

		constexpr auto operator[](i32 p_index) -> ColType &
		{
			TSM_ASSERT_LENGTH(p_index, s_dim);
			return data[p_index];
		}

		constexpr auto operator[](i32 p_index) const -> const ColType &
		{
			TSM_ASSERT_LENGTH(p_index, s_dim);
			return data[p_index];
		}

		constexpr auto determinant() const -> Type { return tsm::determinant(*this); }
	};

	template<typename Type>
	constexpr auto operator*(const Mat3<Type> &p_m, Type p_s) -> Mat3<Type> { return Mat3{p_m[0] * p_s, p_m[1] * p_s, p_m[2] * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const Mat3<Type> &p_m) -> Mat3<Type> { return Mat3{p_s * p_m[0], p_s * p_m[1], p_s * p_m[2]}; }

	template<typename Type>
	constexpr auto operator*(const Mat3<Type> &p_m, typename Mat3<Type>::ColType p_v) -> Mat3<Type>::ColType
	{
		return typename Mat3<Type>::ColType{
			p_m[0][0] * p_v.x + p_m[1][0] * p_v.y + p_m[2][0] * p_v.z,
			p_m[0][1] * p_v.x + p_m[1][1] * p_v.y + p_m[2][1] * p_v.z,
			p_m[0][2] * p_v.x + p_m[1][2] * p_v.y + p_m[2][2] * p_v.z
		};
	}

	template<typename Type>
	constexpr auto operator*(typename Mat3<Type>::ColType p_v, const Mat3<Type> &p_m) -> Mat3<Type>::ColType
	{
		return typename Mat3<Type>::ColType{
			p_v.x * p_m[0][0] + p_v.y * p_m[1][0] + p_v.z * p_m[2][0],
			p_v.x * p_m[0][1] + p_v.y * p_m[1][1] + p_v.z * p_m[2][1],
			p_v.x * p_m[0][2] + p_v.y * p_m[1][2] + p_v.z * p_m[2][2]
		};
	}

	template<typename Type>
	constexpr auto operator*(const Mat3<Type> &p_m1, const Mat3<Type> &p_m2) -> Mat3<Type>
	{
		return Mat3<Type>{
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
	constexpr auto determinant(const Mat3<Type> &p_m) -> Type
	{
		Type d1{p_m[0][0] * (p_m[1][1] * p_m[2][2] - p_m[2][1] * p_m[1][2])};
		Type d2{p_m[1][0] * (p_m[0][1] * p_m[2][2] - p_m[2][1] * p_m[0][2])};
		Type d3{p_m[2][0] * (p_m[0][1] * p_m[1][2] - p_m[1][1] * p_m[0][2])};
		return {d1 - d2 + d3};
	}

	template<typename Type>
	constexpr auto inverse(const Mat3<Type> &p_m) -> Mat3<Type>
	{
		Type one_over_determinant{
			static_cast<Type>(1) / (+p_m[0][0] * (p_m[1][1] * p_m[2][2] - p_m[2][1] * p_m[1][2]) - p_m[1][0] * (p_m[0][1] * p_m[2][2] - p_m[2][1] * p_m[0][2]) + p_m[2][0]
									* (p_m[0][1] * p_m[1][2] - p_m[1][1] * p_m[0][2]))
		};

		Mat3<Type> inverse;
		inverse[0][0] = +(p_m[1][1] * p_m[2][2] - p_m[2][1] * p_m[1][2]);
		inverse[1][0] = -(p_m[1][0] * p_m[2][2] - p_m[2][0] * p_m[1][2]);
		inverse[2][0] = +(p_m[1][0] * p_m[2][1] - p_m[2][0] * p_m[1][1]);
		inverse[0][1] = -(p_m[0][1] * p_m[2][2] - p_m[2][1] * p_m[0][2]);
		inverse[1][1] = +(p_m[0][0] * p_m[2][2] - p_m[2][0] * p_m[0][2]);
		inverse[2][1] = -(p_m[0][0] * p_m[2][1] - p_m[2][0] * p_m[0][1]);
		inverse[0][2] = +(p_m[0][1] * p_m[1][2] - p_m[1][1] * p_m[0][2]);
		inverse[1][2] = -(p_m[0][0] * p_m[1][2] - p_m[1][0] * p_m[0][2]);
		inverse[2][2] = +(p_m[0][0] * p_m[1][1] - p_m[1][0] * p_m[0][1]);

		inverse = inverse * one_over_determinant;
		return inverse;
	}

	template<typename Type>
	constexpr auto transpose(const Mat3<Type> &p_m) -> Mat3<Type>
	{
		Mat3<Type> result{static_cast<Type>(1)};
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
}
