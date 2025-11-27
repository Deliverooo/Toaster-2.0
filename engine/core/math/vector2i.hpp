#pragma once

#include "string/string.hpp"
#include "core_typedefs.hpp"

#include "math/math_constants.hpp"
#include "error_macros.hpp"

namespace tst
{
	struct Vector2;

	struct Vector2I
	{
		static constexpr int32 AXIS_COUNT = 2;

		union
		{
			struct
			{
				int32 x;
				int32 y;
			};

			int32 data[AXIS_COUNT] = {};
		};

		_ALWAYS_INLINE_ int32 &operator[](const int axis)
		{
			TST_ASSERT(axis < AXIS_COUNT);
			return data[axis];
		}

		_ALWAYS_INLINE_ const int32 &operator[](const int axis) const
		{
			TST_ASSERT(axis < AXIS_COUNT);
			return data[axis];
		}

		constexpr Vector2I operator+(Vector2I val) const;
		constexpr void     operator+=(const Vector2I &val);
		constexpr Vector2I operator-(const Vector2I &val) const;
		constexpr void     operator-=(const Vector2I &val);
		constexpr Vector2I operator*(const Vector2I &val) const;

		constexpr Vector2I operator*(int32 val) const;
		constexpr void     operator*=(int32 val);
		constexpr void     operator*=(const Vector2I &val) { *this = *this * val; }

		constexpr Vector2I operator/(const Vector2I &val) const;

		constexpr Vector2I operator/(int32 val) const;

		constexpr void operator/=(int32 val);
		constexpr void operator/=(const Vector2I &val) { *this = *this / val; }

		constexpr Vector2I operator-() const;

		constexpr bool operator==(const Vector2I &val) const;
		constexpr bool operator!=(const Vector2I &val) const;

		explicit                  operator std::string() const;
		[[nodiscard]] std::string to_string() const;

		[[nodiscard]] float32 length() const;

		[[nodiscard]] float32 dot(const Vector2I &v) const;
		[[nodiscard]] float32 angle(const Vector2I &v) const;

		Vector2I clamp(const Vector2I &min, const Vector2I &max) const;
		Vector2I clamp(int32 min, int32 max) const;

		constexpr Vector2I() : x(0), y(0)
		{
		}

		constexpr Vector2I(int32 x, int32 y) : x(x), y(y)
		{
		}

		operator Vector2() const;
	};

	using Size2I = Vector2I;
}
