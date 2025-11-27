#pragma once

#include "string/string.hpp"
#include "core_typedefs.hpp"

#include "math/math_constants.hpp"
#include "error_macros.hpp"

namespace tst
{
	struct TST_CORE_API Vector4
	{
		static constexpr int32 AXIS_COUNT = 4;

		union
		{
			struct
			{
				float32 x;
				float32 y;
				float32 z;
				float32 w;
			};

			float32 data[AXIS_COUNT] = {};
		};

		_ALWAYS_INLINE_ float32 &operator[](const int axis)
		{
			TST_ASSERT(axis < AXIS_COUNT);
			return data[axis];
		}

		_ALWAYS_INLINE_ const float32 &operator[](const int axis) const
		{
			TST_ASSERT(axis < AXIS_COUNT);
			return data[axis];
		}

		constexpr Vector4 operator+(Vector4 val) const;
		constexpr void    operator+=(const Vector4 &val);
		constexpr Vector4 operator-(const Vector4 &val) const;
		constexpr void    operator-=(const Vector4 &val);
		constexpr Vector4 operator*(const Vector4 &val) const;

		constexpr Vector4 operator*(float32 val) const;
		constexpr void    operator*=(float32 val);
		constexpr void    operator*=(const Vector4 &val) { *this = *this * val; }

		constexpr Vector4 operator/(const Vector4 &val) const;

		constexpr Vector4 operator/(float32 val) const;

		constexpr void operator/=(float32 val);
		constexpr void operator/=(const Vector4 &val) { *this = *this / val; }

		constexpr Vector4 operator-() const;

		constexpr bool operator==(const Vector4 &val) const;
		constexpr bool operator!=(const Vector4 &val) const;

		explicit             operator String() const;
		[[nodiscard]] String to_string() const;

		void                  normalize();
		[[nodiscard]] bool    isNormalized() const;
		[[nodiscard]] Vector4 normalized() const;

		[[nodiscard]] float32 length() const;

		[[nodiscard]] float32 dot(const Vector4 &v) const;
		[[nodiscard]] float32 angle(const Vector4 &v) const;
	};
}
