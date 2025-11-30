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

#include "core_typedefs.hpp"
#include "core_defines.hpp"
#include "math_constants.hpp"

#include <cmath>
#include <utility>
#undef min
#undef max


namespace tst::math
{
	_ALWAYS_INLINE_ float64 sin(float64 val)
	{
		return std::sin(val);
	}

	_ALWAYS_INLINE_ float32 sin(float32 val)
	{
		return std::sin(val);
	}

	_ALWAYS_INLINE_ float64 cos(float64 val)
	{
		return std::cos(val);
	}

	_ALWAYS_INLINE_ float32 cos(float32 val)
	{
		return std::cos(val);
	}

	_ALWAYS_INLINE_ float64 tan(float64 val)
	{
		return std::tan(val);
	}

	_ALWAYS_INLINE_ float32 tan(float32 val)
	{
		return std::tan(val);
	}

	_ALWAYS_INLINE_ float64 sinh(float64 val)
	{
		return std::sinh(val);
	}

	_ALWAYS_INLINE_ float32 sinh(float32 val)
	{
		return std::sinh(val);
	}

	_ALWAYS_INLINE_ float64 cosh(float64 val)
	{
		return std::cosh(val);
	}

	_ALWAYS_INLINE_ float32 cosh(float32 val)
	{
		return std::cosh(val);
	}

	_ALWAYS_INLINE_ float64 tanh(float64 val)
	{
		return std::tanh(val);
	}

	_ALWAYS_INLINE_ float32 tanh(float32 val)
	{
		return std::tanh(val);
	}

	_ALWAYS_INLINE_ float64 asin(float64 val)
	{
		return std::asin(val);
	}

	_ALWAYS_INLINE_ float32 asin(float32 val)
	{
		return std::asin(val);
	}

	_ALWAYS_INLINE_ float64 acos(float64 val)
	{
		return std::acos(val);
	}

	_ALWAYS_INLINE_ float32 acos(float32 val)
	{
		return std::acos(val);
	}

	_ALWAYS_INLINE_ float64 atan(float64 val)
	{
		return std::atan(val);
	}

	_ALWAYS_INLINE_ float32 atan(float32 val)
	{
		return std::atan(val);
	}

	_ALWAYS_INLINE_ float64 sqrt(float64 val)
	{
		return std::sqrt(val);
	}

	_ALWAYS_INLINE_ float32 sqrt(float32 val)
	{
		return std::sqrt(val);
	}

	_ALWAYS_INLINE_ float64 fmod(float64 val, float64 p_y)
	{
		return std::fmod(val, p_y);
	}

	_ALWAYS_INLINE_ float32 fmod(float32 val, float32 p_y)
	{
		return std::fmod(val, p_y);
	}

	_ALWAYS_INLINE_ float64 modf(float64 val, float64 *r_y)
	{
		return std::modf(val, r_y);
	}

	_ALWAYS_INLINE_ float32 modf(float32 val, float32 *r_y)
	{
		return std::modf(val, r_y);
	}

	_ALWAYS_INLINE_ float64 floor(float64 val)
	{
		return std::floor(val);
	}

	_ALWAYS_INLINE_ float32 floor(float32 val)
	{
		return std::floor(val);
	}

	_ALWAYS_INLINE_ float64 ceil(float64 val)
	{
		return std::ceil(val);
	}

	_ALWAYS_INLINE_ float32 ceil(float32 val)
	{
		return std::ceil(val);
	}

	_ALWAYS_INLINE_ float64 pow(float64 val, float64 p_y)
	{
		return std::pow(val, p_y);
	}

	_ALWAYS_INLINE_ float32 pow(float32 val, float32 p_y)
	{
		return std::pow(val, p_y);
	}

	_ALWAYS_INLINE_ float64 log(float64 val)
	{
		return std::log(val);
	}

	_ALWAYS_INLINE_ float32 log(float32 val)
	{
		return std::log(val);
	}

	_ALWAYS_INLINE_ float64 log2(float64 val)
	{
		return std::log2(val);
	}

	_ALWAYS_INLINE_ float32 log2(float32 val)
	{
		return std::log2(val);
	}

	_ALWAYS_INLINE_ float64 exp(float64 val)
	{
		return std::exp(val);
	}

	_ALWAYS_INLINE_ float32 exp(float32 val)
	{
		return std::exp(val);
	}

	_ALWAYS_INLINE_ float64 abs(float64 val)
	{
		return std::abs(val);
	}

	_ALWAYS_INLINE_ float32 abs(float32 val)
	{
		return std::abs(val);
	}

	_ALWAYS_INLINE_ float64 deg_to_rad(float64 val)
	{
		return val * (constants::pi<float32>() / 180.0);
	}

	_ALWAYS_INLINE_ float32 deg_to_rad(float32 val)
	{
		return val * (constants::pi<float32>() / 180.0f);
	}

	_ALWAYS_INLINE_ float64 rad_to_deg(float64 val)
	{
		return val * (180.0 / constants::pi<float32>());
	}

	_ALWAYS_INLINE_ float32 rad_to_deg(float32 val)
	{
		return val * (180.0f / constants::pi<float32>());
	}

	_ALWAYS_INLINE_ float64 lerp(float64 from, float64 to, float64 weight)
	{
		return from + (to - from) * weight;
	}

	_ALWAYS_INLINE_ float32 lerp(float32 from, float32 to, float32 weight)
	{
		return from + (to - from) * weight;
	}

	_ALWAYS_INLINE_ float64 round(float64 val)
	{
		return std::round(val);
	}

	_ALWAYS_INLINE_ float32 round(float32 val)
	{
		return std::round(val);
	}

	template<typename T>
	T min(T lhs, T rhs)
	{
		return std::min(lhs, rhs);
	}

	template<typename T>
	T max(T lhs, T rhs)
	{
		return std::max(lhs, rhs);
	}

	template<typename T, typename T2, typename T3>
	constexpr auto clamp(const T m_a, const T2 m_min, const T3 m_max)
	{
		return m_a < m_min ? m_min : (m_a > m_max ? m_max : m_a);
	}
}
