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

#include "core_defines.hpp"
#include "core_typedefs.hpp"

namespace tst
{

	namespace math::constants
	{
		template<real_c T>
		_ALWAYS_INLINE_ constexpr T pi()
		{
			return static_cast<T>(3.14159265358979323846264338327950288);
		}

		template<real_c T>
		_ALWAYS_INLINE_ constexpr T tau()
		{
			return static_cast<T>(6.2831853071795864769252867666);
		}

		template<real_c T>
		_ALWAYS_INLINE_ constexpr T e()
		{
			return static_cast<T>(2.7182818284590452353602874714);
		}

		template<real_c T>
		_ALWAYS_INLINE_ constexpr T phi()
		{
			return static_cast<T>(1.61803398875);
		}

		template<real_c T>
		_ALWAYS_INLINE_ constexpr T sqrt2()
		{
			return static_cast<T>(1.4142135623730950488016887488);
		}

		template<real_c T>
		_ALWAYS_INLINE_ constexpr T sqrt3()
		{
			return static_cast<T>(1.7320508075688772935274463415059);
		}

		template<real_c T>
		_ALWAYS_INLINE_ constexpr T ln2()
		{
			return static_cast<T>(0.6931471805599453094172321215);
		}
	}
}
