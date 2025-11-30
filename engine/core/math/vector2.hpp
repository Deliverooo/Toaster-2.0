/*
*  	Copyright 2025 Orbo Stetson
 *  	Licensed under the Apache License, Version 2.0 (the "License");
 *  	you may not use this file except in compliance with the License.
 *  	You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */
#pragma once

#include "string/string.hpp"

#include "core_typedefs.hpp"
#include "math/math_constants.hpp"
#include "error_macros.hpp"

namespace tst
{
	struct Vector2I;

	struct TST_CORE_API Vector2
	{
		static constexpr int32 AXIS_COUNT = 2;

		union
		{
			struct
			{
				float32 x;
				float32 y;
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

		constexpr Vector2() : x(0.0f), y(0.0f)
		{
		}

		constexpr Vector2(float32 x, float32 y) : x(x), y(y)
		{
		}

		constexpr Vector2 operator+(const Vector2 &val) const;
		constexpr void    operator+=(const Vector2 &val);
		constexpr Vector2 operator-(const Vector2 &val) const;
		constexpr void    operator-=(const Vector2 &val);
		constexpr Vector2 operator*(const Vector2 &val) const;

		constexpr Vector2 operator*(float32 val) const;
		constexpr void    operator*=(float32 val);
		constexpr void    operator*=(const Vector2 &val) { *this = *this * val; }

		constexpr Vector2 operator/(const Vector2 &val) const;

		constexpr Vector2 operator/(float32 val) const;

		constexpr void operator/=(float32 val);
		constexpr void operator/=(const Vector2 &val) { *this = *this / val; }

		constexpr Vector2 operator-() const;

		constexpr bool operator==(const Vector2 &val) const;
		constexpr bool operator!=(const Vector2 &val) const;

		explicit             operator String() const;
		[[nodiscard]] String to_string() const;

		void                  normalize();
		[[nodiscard]] bool    isNormalized() const;
		[[nodiscard]] Vector2 normalized() const;

		[[nodiscard]] float32 length() const;

		[[nodiscard]] float32 dot(const Vector2 &v) const;
		[[nodiscard]] float32 angle(const Vector2 &v) const;

		[[nodiscard]] Vector2 min(const Vector2 &v) const;
		[[nodiscard]] Vector2 max(const Vector2 &v) const;

		operator Vector2I() const;
	};

	constexpr Vector2 Vector2::operator+(const Vector2 &val) const
	{
		return {x + val.x, y + val.y};
	}

	constexpr void Vector2::operator+=(const Vector2 &val)
	{
		x += val.x;
		y += val.y;
	}

	constexpr Vector2 Vector2::operator-(const Vector2 &val) const
	{
		return {x - val.x, y - val.y};
	}

	constexpr void Vector2::operator-=(const Vector2 &val)
	{
		x -= val.x;
		y -= val.y;
	}

	constexpr Vector2 Vector2::operator*(const Vector2 &val) const
	{
		return {x * val.x, y * val.y};
	}

	constexpr Vector2 Vector2::operator*(float32 val) const
	{
		return {x * val, y * val};
	}

	constexpr void Vector2::operator*=(float32 val)
	{
		x *= val;
		y *= val;
	}

	constexpr Vector2 Vector2::operator/(const Vector2 &val) const
	{
		return {x / val.x, y / val.y};
	}

	constexpr Vector2 Vector2::operator/(float32 val) const
	{
		return {x / val, y / val};
	}

	constexpr void Vector2::operator/=(float32 val)
	{
		x /= val;
		y /= val;
	}

	constexpr Vector2 Vector2::operator-() const
	{
		return {-x, -y};
	}

	constexpr bool Vector2::operator==(const Vector2 &val) const
	{
		return x == val.x && y == val.y;
	}

	constexpr bool Vector2::operator!=(const Vector2 &val) const
	{
		return x != val.x || y != val.y;
	}

	inline Vector2::operator String() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + "]"};
	}

	using Size2 = Vector2;
}
