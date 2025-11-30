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

		constexpr Vector2I operator+(const Vector2I &val) const;
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

		[[nodiscard]] Vector2I min(const Vector2I &v) const;
		[[nodiscard]] Vector2I max(const Vector2I &v) const;

		[[nodiscard]] Vector2I clamp(const Vector2I &min, const Vector2I &max) const;
		[[nodiscard]] Vector2I clamp(int32 min, int32 max) const;

		constexpr Vector2I() : x(0), y(0)
		{
		}

		constexpr Vector2I(int32 x, int32 y) : x(x), y(y)
		{
		}

		operator Vector2() const;
	};
	constexpr Vector2I Vector2I::operator+(const Vector2I &val) const
	{
		return {x + val.x, y + val.y};
	}

	constexpr void Vector2I::operator+=(const Vector2I &val)
	{
		x += val.x;
		y += val.y;
	}

	constexpr Vector2I Vector2I::operator-(const Vector2I &val) const
	{
		return {x - val.x, y - val.y};
	}

	constexpr void Vector2I::operator-=(const Vector2I &val)
	{
		x -= val.x;
		y -= val.y;
	}

	constexpr Vector2I Vector2I::operator*(const Vector2I &val) const
	{
		return {x * val.x, y * val.y};
	}

	constexpr Vector2I Vector2I::operator*(int32 val) const
	{
		return {x * val, y * val};
	}

	constexpr void Vector2I::operator*=(int32 val)
	{
		x *= val;
		y *= val;
	}

	constexpr Vector2I Vector2I::operator/(const Vector2I &val) const
	{
		return {x / val.x, y / val.y};
	}

	constexpr Vector2I Vector2I::operator/(int32 val) const
	{
		return {x / val, y / val};
	}

	constexpr void Vector2I::operator/=(int32 val)
	{
		x /= val;
		y /= val;
	}

	constexpr Vector2I Vector2I::operator-() const
	{
		return {-x, -y};
	}

	constexpr bool Vector2I::operator==(const Vector2I &val) const
	{
		return x == val.x && y == val.y;
	}

	constexpr bool Vector2I::operator!=(const Vector2I &val) const
	{
		return x != val.x || y != val.y;
	}

	inline Vector2I::operator String() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + "]"};
	}


	using Size2I = Vector2I;
}
