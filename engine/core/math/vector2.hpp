#pragma once

#include <string>

#include "math/math_types.hpp"
#include "error_macros.hpp"

namespace tst
{
	struct TST_CORE_API Vector2
	{
		static constexpr Int AXIS_COUNT = 2;

		union
		{
			struct
			{
				Float x;
				Float y;
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

		constexpr Vector2 operator+(Vector2 val) const;
		constexpr void    operator+=(const Vector2 &val);
		constexpr Vector2 operator-(const Vector2 &val) const;
		constexpr void    operator-=(const Vector2 &val);
		constexpr Vector2 operator*(const Vector2 &val) const;

		constexpr Vector2 operator*(Float val) const;
		constexpr void    operator*=(Float val);
		constexpr void    operator*=(const Vector2 &val) { *this = *this * val; }

		constexpr Vector2 operator/(const Vector2 &val) const;

		constexpr Vector2 operator/(Float val) const;

		constexpr void operator/=(Float val);
		constexpr void operator/=(const Vector2 &val) { *this = *this / val; }

		constexpr Vector2 operator-() const;

		constexpr bool operator==(const Vector2 &val) const;
		constexpr bool operator!=(const Vector2 &val) const;

		explicit                  operator std::string() const;
		[[nodiscard]] std::string to_string() const;

		void                  normalize();
		[[nodiscard]] bool    isNormalized() const;
		[[nodiscard]] Vector2 normalized() const;

		[[nodiscard]] Float length() const;

		[[nodiscard]] Float dot(const Vector2 &v) const;
		[[nodiscard]] Float angle(const Vector2 &v) const;
	};
}
