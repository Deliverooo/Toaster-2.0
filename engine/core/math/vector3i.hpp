#pragma once

#include <string>

#include "math/math_types.hpp"
#include "error_macros.hpp"

namespace tst
{
	struct TST_CORE_API Vector3I
	{
		static constexpr Int AXIS_COUNT = 3;

		union
		{
			struct
			{
				Int x;
				Int y;
				Int z;
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

		constexpr Vector3I operator+(Vector3I val) const;
		constexpr void     operator+=(const Vector3I &val);
		constexpr Vector3I operator-(const Vector3I &val) const;
		constexpr void     operator-=(const Vector3I &val);
		constexpr Vector3I operator*(const Vector3I &val) const;

		constexpr Vector3I operator*(Int val) const;
		constexpr void     operator*=(Int val);
		constexpr void     operator*=(const Vector3I &val) { *this = *this * val; }

		constexpr Vector3I operator/(const Vector3I &val) const;

		constexpr Vector3I operator/(Int val) const;

		constexpr void operator/=(Int val);
		constexpr void operator/=(const Vector3I &val) { *this = *this / val; }

		constexpr Vector3I operator-() const;

		constexpr bool operator==(const Vector3I &val) const;
		constexpr bool operator!=(const Vector3I &val) const;

		explicit                  operator std::string() const;
		[[nodiscard]] std::string to_string() const;

		[[nodiscard]] Float length() const;

		[[nodiscard]] Float dot(const Vector3I &v) const;
		[[nodiscard]] Float angle(const Vector3I &v) const;
	};
	using Size3I = Vector3I;

}
