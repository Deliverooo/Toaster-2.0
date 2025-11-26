#pragma once

#include "error_macros.hpp"
#include <cmath>

#include "math_types.hpp"

namespace tst::math
{
	_ALWAYS_INLINE_ double sin(double val)
	{
		return std::sin(val);
	}

	_ALWAYS_INLINE_ float sin(float val)
	{
		return std::sin(val);
	}

	_ALWAYS_INLINE_ double cos(double val)
	{
		return std::cos(val);
	}

	_ALWAYS_INLINE_ float cos(float val)
	{
		return std::cos(val);
	}

	_ALWAYS_INLINE_ double tan(double val)
	{
		return std::tan(val);
	}

	_ALWAYS_INLINE_ float tan(float val)
	{
		return std::tan(val);
	}

	_ALWAYS_INLINE_ double sinh(double val)
	{
		return std::sinh(val);
	}

	_ALWAYS_INLINE_ float sinh(float val)
	{
		return std::sinh(val);
	}

	_ALWAYS_INLINE_ double cosh(double val)
	{
		return std::cosh(val);
	}

	_ALWAYS_INLINE_ float cosh(float val)
	{
		return std::cosh(val);
	}

	_ALWAYS_INLINE_ double tanh(double val)
	{
		return std::tanh(val);
	}

	_ALWAYS_INLINE_ float tanh(float val)
	{
		return std::tanh(val);
	}

	_ALWAYS_INLINE_ double asin(double val)
	{
		return std::asin(val);
	}

	_ALWAYS_INLINE_ float asin(float val)
	{
		return std::asin(val);
	}

	_ALWAYS_INLINE_ double acos(double val)
	{
		return std::acos(val);
	}

	_ALWAYS_INLINE_ float acos(float val)
	{
		return std::acos(val);
	}

	_ALWAYS_INLINE_ double atan(double val)
	{
		return std::atan(val);
	}

	_ALWAYS_INLINE_ float atan(float val)
	{
		return std::atan(val);
	}

	_ALWAYS_INLINE_ double sqrt(double val)
	{
		return std::sqrt(val);
	}

	_ALWAYS_INLINE_ float sqrt(float val)
	{
		return std::sqrt(val);
	}

	_ALWAYS_INLINE_ double fmod(double val, double p_y)
	{
		return std::fmod(val, p_y);
	}

	_ALWAYS_INLINE_ float fmod(float val, float p_y)
	{
		return std::fmod(val, p_y);
	}

	_ALWAYS_INLINE_ double modf(double val, double *r_y)
	{
		return std::modf(val, r_y);
	}

	_ALWAYS_INLINE_ float modf(float val, float *r_y)
	{
		return std::modf(val, r_y);
	}

	_ALWAYS_INLINE_ double floor(double val)
	{
		return std::floor(val);
	}

	_ALWAYS_INLINE_ float floor(float val)
	{
		return std::floor(val);
	}

	_ALWAYS_INLINE_ double ceil(double val)
	{
		return std::ceil(val);
	}

	_ALWAYS_INLINE_ float ceil(float val)
	{
		return std::ceil(val);
	}

	_ALWAYS_INLINE_ double pow(double val, double p_y)
	{
		return std::pow(val, p_y);
	}

	_ALWAYS_INLINE_ float pow(float val, float p_y)
	{
		return std::pow(val, p_y);
	}

	_ALWAYS_INLINE_ double log(double val)
	{
		return std::log(val);
	}

	_ALWAYS_INLINE_ float log(float val)
	{
		return std::log(val);
	}

	_ALWAYS_INLINE_ double log2(double val)
	{
		return std::log2(val);
	}

	_ALWAYS_INLINE_ float log2(float val)
	{
		return std::log2(val);
	}

	_ALWAYS_INLINE_ double exp(double val)
	{
		return std::exp(val);
	}

	_ALWAYS_INLINE_ float exp(float val)
	{
		return std::exp(val);
	}

	_ALWAYS_INLINE_ double abs(double val)
	{
		return std::abs(val);
	}

	_ALWAYS_INLINE_ float abs(float val)
	{
		return std::abs(val);
	}

	_ALWAYS_INLINE_ double deg_to_rad(double val)
	{
		return val * (constants::pi<float>() / 180.0);
	}

	_ALWAYS_INLINE_ float deg_to_rad(float val)
	{
		return val * (constants::pi<float>() / 180.0f);
	}

	_ALWAYS_INLINE_ double rad_to_deg(double val)
	{
		return val * (180.0 / constants::pi<float>());
	}

	_ALWAYS_INLINE_ float rad_to_deg(float val)
	{
		return val * (180.0f / constants::pi<float>());
	}

	_ALWAYS_INLINE_ double lerp(double from, double to, double weight)
	{
		return from + (to - from) * weight;
	}

	_ALWAYS_INLINE_ float lerp(float from, float to, float weight)
	{
		return from + (to - from) * weight;
	}

	_ALWAYS_INLINE_ double round(double val)
	{
		return std::round(val);
	}

	_ALWAYS_INLINE_ float round(float val)
	{
		return std::round(val);
	}
}
