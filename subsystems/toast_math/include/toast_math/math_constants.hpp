#pragma once

#include "types.hpp"

#include <concepts>
#include <limits>

namespace tsm::constants
{
	template<typename Type> requires std::floating_point<Type>
	constexpr auto pi() -> Type { return static_cast<Type>(3.141592653589793284); }

	template<typename Type> requires std::floating_point<Type>
	constexpr auto sqrt2() -> Type { return static_cast<Type>(1.414213562373095048801688724288); }

	template<typename Type>
	constexpr auto epsilon() -> Type { return std::numeric_limits<Type>::epsilon(); }
}
