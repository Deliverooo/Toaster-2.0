#pragma once

#include <string>

#include "math/math_types.hpp"
#include "error_macros.hpp"

namespace tst
{
	struct Vector2I
	{
		static constexpr Int AXIS_COUNT = 2;

		union
		{
			struct
			{
				Int x;
				Int y;
			};

			Int data[AXIS_COUNT] = {};
		};

		_ALWAYS_INLINE_ Int &operator[](const int axis)
		{
			TST_ASSERT(axis < AXIS_COUNT);
			return data[axis];
		}

		_ALWAYS_INLINE_ const Int &operator[](const int axis) const
		{
			TST_ASSERT(axis < AXIS_COUNT);
			return data[axis];
		}

		constexpr Vector2I operator+(Vector2I val) const;
		constexpr void     operator+=(const Vector2I &val);
		constexpr Vector2I operator-(const Vector2I &val) const;
		constexpr void     operator-=(const Vector2I &val);
		constexpr Vector2I operator*(const Vector2I &val) const;

		constexpr Vector2I operator*(Int val) const;
		constexpr void     operator*=(Int val);
		constexpr void     operator*=(const Vector2I &val) { *this = *this * val; }

		constexpr Vector2I operator/(const Vector2I &val) const;

		constexpr Vector2I operator/(Int val) const;

		constexpr void operator/=(Int val);
		constexpr void operator/=(const Vector2I &val) { *this = *this / val; }

		constexpr Vector2I operator-() const;

		constexpr bool operator==(const Vector2I &val) const;
		constexpr bool operator!=(const Vector2I &val) const;

		explicit                  operator std::string() const;
		[[nodiscard]] std::string to_string() const;

		[[nodiscard]] Float length() const;

		[[nodiscard]] Float dot(const Vector2I &v) const;
		[[nodiscard]] Float angle(const Vector2I &v) const;
	};

	using Size2I = Vector2I;
}
