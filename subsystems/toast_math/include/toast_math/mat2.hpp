#pragma once

#include "math_vector.hpp"

namespace tsm
{
	template<typename Type>
	struct Mat2;
	template<typename Type>
	struct Mat3;
	template<typename Type>
	struct Mat4;

	template<typename Type>
	constexpr auto determinant(const Mat2<Type> &p_m) -> Type;
	template<typename Type>
	constexpr auto determinant(const Mat3<Type> &p_m) -> Type;
	template<typename Type>
	constexpr auto determinant(const Mat4<Type> &p_m) -> Type;

	template<typename Type>
	constexpr auto inverse(const Mat2<Type> &p_m) -> Mat2<Type>;
	template<typename Type>
	constexpr auto inverse(const Mat3<Type> &p_m) -> Mat3<Type>;
	template<typename Type>
	constexpr auto inverse(const Mat4<Type> &p_m) -> Mat4<Type>;

	template<typename Type>
	constexpr auto transpose(const Mat2<Type> &p_m) -> Mat2<Type>;
	template<typename Type>
	constexpr auto transpose(const Mat3<Type> &p_m) -> Mat3<Type>;
	template<typename Type>
	constexpr auto transpose(const Mat4<Type> &p_m) -> Mat4<Type>;

	template<typename Type>
	struct Mat2
	{
		using ColType = Vec2<Type>;

		static constexpr u32 s_dim{2u};

		ColType data[s_dim];

		constexpr Mat2() = default;

		constexpr Mat2(Type p_m00, Type p_m01, Type p_m10, Type p_m11) : data{ColType{p_m00, p_m10}, ColType{p_m01, p_m11}}
		{
		}

		constexpr Mat2(ColType p_v1, ColType p_v2) : data{p_v1, p_v2}
		{
		}

		constexpr Mat2(Type p_s) : data{{p_s, static_cast<Type>(0)}, {static_cast<Type>(0), p_s}}
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
	constexpr auto operator*(const Mat2<Type> &p_m, Type p_s) -> Mat2<Type> { return Mat2{p_m[0] * p_s, p_m[1] * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const Mat2<Type> &p_m) -> Mat2<Type> { return Mat2{p_s * p_m[0], p_s * p_m[1]}; }

	template<typename Type>
	constexpr auto operator*(const Mat2<Type> &p_m, typename Mat2<Type>::ColType p_v) -> Mat2<Type>::ColType
	{
		return typename Mat2<Type>::ColType{p_m[0][0] * p_v.x + p_m[1][0] * p_v.y, p_m[0][1] * p_v.x + p_m[1][1] * p_v.y};
	}

	template<typename Type>
	constexpr auto operator*(typename Mat2<Type>::ColType p_v, const Mat2<Type> &p_m) -> Mat2<Type>::ColType
	{
		return typename Mat2<Type>::ColType{p_v.x * p_m[0][0] + p_v.y * p_m[0][1], p_v.x * p_m[1][0] + p_v.y * p_m[1][1]};
	}

	template<typename Type>
	constexpr auto operator*(const Mat2<Type> &p_m1, const Mat2<Type> &p_m2) -> Mat2<Type>
	{
		return Mat2<Type>{
			p_m1[0][0] * p_m2[0][0] + p_m1[1][0] * p_m2[0][1],
			p_m1[0][1] * p_m2[0][0] + p_m1[1][1] * p_m2[0][1],
			p_m1[0][0] * p_m2[1][0] + p_m1[1][0] * p_m2[1][1],
			p_m1[0][1] * p_m2[1][0] + p_m1[1][1] * p_m2[1][1]
		};
	}

	template<typename Type>
	constexpr auto determinant(const Mat2<Type> &p_m) -> Type
	{
		return {p_m[0][0] * p_m[1][1] - p_m[1][0] * p_m[0][1]};
	}

	template<typename Type>
	constexpr auto inverse(const Mat2<Type> &p_m) -> Mat2<Type>
	{
		Type one_over_determinant{static_cast<Type>(1) / deternimant(p_m)};
		Mat2 Inverse{p_m[1][1] * one_over_determinant, -p_m[0][1] * one_over_determinant, -p_m[1][0] * one_over_determinant, p_m[0][0] * one_over_determinant};
		return Inverse;
	}

	template<typename Type>
	constexpr auto transpose(const Mat2<Type> &p_m) -> Mat2<Type>
	{
		Mat2<Type> result{static_cast<Type>(1)};
		result[0][0] = p_m[0][0];
		result[0][1] = p_m[1][0];
		result[1][0] = p_m[0][1];
		result[1][1] = p_m[1][1];
		return result;
	}
}
