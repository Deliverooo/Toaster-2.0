#pragma once
#include <concepts>

namespace tsm::constants
{
	template<typename T> requires std::floating_point<T>
	constexpr T pi()
	{
		return T(3.14159265358979323846264338327950288);
	}

	template<typename T> requires std::floating_point<T>
	constexpr T sqrt2()
	{
		return T(1.414213562373095048801688724288);
	}
}
