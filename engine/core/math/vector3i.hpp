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
	struct TST_CORE_API Vector3I
	{
		static constexpr int32 AXIS_COUNT = 3;

		union
		{
			struct
			{
				int32 x;
				int32 y;
				int32 z;
			};

			int32 data[AXIS_COUNT] = {};
		};

		_ALWAYS_INLINE_ int32 &operator[](const int32 axis)
		{
			TST_ASSERT(axis < AXIS_COUNT);
			return data[axis];
		}

		_ALWAYS_INLINE_ const int32 &operator[](const int32 axis) const
		{
			TST_ASSERT(axis < AXIS_COUNT);
			return data[axis];
		}

		constexpr Vector3I operator+(Vector3I val) const;
		constexpr void     operator+=(const Vector3I &val);
		constexpr Vector3I operator-(const Vector3I &val) const;
		constexpr void     operator-=(const Vector3I &val);
		constexpr Vector3I operator*(const Vector3I &val) const;

		constexpr Vector3I operator*(int32 val) const;
		constexpr void     operator*=(int32 val);
		constexpr void     operator*=(const Vector3I &val) { *this = *this * val; }

		constexpr Vector3I operator/(const Vector3I &val) const;

		constexpr Vector3I operator/(int32 val) const;

		constexpr void operator/=(int32 val);
		constexpr void operator/=(const Vector3I &val) { *this = *this / val; }

		constexpr Vector3I operator-() const;

		constexpr bool operator==(const Vector3I &val) const;
		constexpr bool operator!=(const Vector3I &val) const;

		explicit             operator String() const;
		[[nodiscard]] String to_string() const;

		[[nodiscard]] float32 length() const;

		[[nodiscard]] float32 dot(const Vector3I &v) const;
		[[nodiscard]] float32 angle(const Vector3I &v) const;
	};

	using Size3I = Vector3I;
}
