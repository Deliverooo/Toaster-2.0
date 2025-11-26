#pragma once

#include <string>

#include "math/math_types.hpp"
#include "error_macros.hpp"

namespace tst
{
	struct TST_CORE_API Vector3
	{
		static constexpr Int AXIS_COUNT = 3;

		union
		{
			struct
			{
				Float x;
				Float y;
				Float z;
			};

			Float data[AXIS_COUNT] = {};
		};

		_ALWAYS_INLINE_ Float &operator[](const int axis)
		{
			TST_ASSERT(axis < AXIS_COUNT);
			return data[axis];
		}

		_ALWAYS_INLINE_ const Float &operator[](const int axis) const
		{
			TST_ASSERT(axis < AXIS_COUNT);
			return data[axis];
		}

		constexpr Vector3 operator+(Vector3 val) const;
		constexpr void    operator+=(const Vector3 &val);
		constexpr Vector3 operator-(const Vector3 &val) const;
		constexpr void    operator-=(const Vector3 &val);
		constexpr Vector3 operator*(const Vector3 &val) const;

		constexpr Vector3 operator*(Float val) const;
		constexpr void    operator*=(Float val);
		constexpr void    operator*=(const Vector3 &val) { *this = *this * val; }

		constexpr Vector3 operator/(const Vector3 &val) const;

		constexpr Vector3 operator/(Float val) const;

		constexpr void operator/=(Float val);
		constexpr void operator/=(const Vector3 &val) { *this = *this / val; }

		constexpr Vector3 operator-() const;

		constexpr bool operator==(const Vector3 &val) const;
		constexpr bool operator!=(const Vector3 &val) const;

		explicit                  operator std::string() const;
		[[nodiscard]] std::string to_string() const;

		void                  normalize();
		[[nodiscard]] bool    isNormalized() const;
		[[nodiscard]] Vector3 normalized() const;

		[[nodiscard]] Float length() const;

		[[nodiscard]] Float dot(const Vector3 &v) const;
		[[nodiscard]] Float angle(const Vector3 &v) const;
	};
}
