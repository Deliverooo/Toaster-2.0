#pragma once

#include <string>

#include "math/math_types.hpp"
#include "error_macros.hpp"

namespace tst
{
	struct TST_CORE_API Vector4I
	{
		static constexpr Int AXIS_COUNT = 4;

		union
		{
			struct
			{
				Int x;
				Int y;
				Int z;
				Int w;
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

		constexpr Vector4I operator+(Vector4I val) const;
		constexpr void     operator+=(const Vector4I &val);
		constexpr Vector4I operator-(const Vector4I &val) const;
		constexpr void     operator-=(const Vector4I &val);
		constexpr Vector4I operator*(const Vector4I &val) const;

		constexpr Vector4I operator*(Int val) const;
		constexpr void     operator*=(Int val);
		constexpr void     operator*=(const Vector4I &val) { *this = *this * val; }

		constexpr Vector4I operator/(const Vector4I &val) const;

		constexpr Vector4I operator/(Int val) const;

		constexpr void operator/=(Int val);
		constexpr void operator/=(const Vector4I &val) { *this = *this / val; }

		constexpr Vector4I operator-() const;

		constexpr bool operator==(const Vector4I &val) const;
		constexpr bool operator!=(const Vector4I &val) const;

		explicit                  operator std::string() const;
		[[nodiscard]] std::string to_string() const;

		[[nodiscard]] Float length() const;

		[[nodiscard]] Float dot(const Vector4I &v) const;
		[[nodiscard]] Float angle(const Vector4I &v) const;
	};

	using Size4I = Vector4I;
}
