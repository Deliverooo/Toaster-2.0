#pragma once

#include "string/string.hpp"
#include "core_typedefs.hpp"

#include "math/math_constants.hpp"
#include "error_macros.hpp"

namespace tst
{
	struct TST_CORE_API Vector3
	{
		static constexpr int32 AXIS_COUNT = 3;

		union
		{
			struct
			{
				float32 x;
				float32 y;
				float32 z;
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

		constexpr Vector3 operator+(Vector3 val) const;
		constexpr void    operator+=(const Vector3 &val);
		constexpr Vector3 operator-(const Vector3 &val) const;
		constexpr void    operator-=(const Vector3 &val);
		constexpr Vector3 operator*(const Vector3 &val) const;

		constexpr Vector3 operator*(float32 val) const;
		constexpr void    operator*=(float32 val);
		constexpr void    operator*=(const Vector3 &val) { *this = *this * val; }

		constexpr Vector3 operator/(const Vector3 &val) const;

		constexpr Vector3 operator/(float32 val) const;

		constexpr void operator/=(float32 val);
		constexpr void operator/=(const Vector3 &val) { *this = *this / val; }

		constexpr Vector3 operator-() const;

		constexpr bool operator==(const Vector3 &val) const;
		constexpr bool operator!=(const Vector3 &val) const;

		explicit                  operator String() const;
		[[nodiscard]] String to_string() const;

		void                  normalize();
		[[nodiscard]] bool    isNormalized() const;
		[[nodiscard]] Vector3 normalized() const;

		[[nodiscard]] float32 length() const;

		[[nodiscard]] float32 dot(const Vector3 &v) const;
		[[nodiscard]] float32 angle(const Vector3 &v) const;
	};
}
