#pragma once
#include <concepts>

namespace tsm::constants
{
	template<typename Type> requires std::floating_point<Type>
	constexpr auto pi() -> Type
	{
		return Type(3.14159265358979323846264338327950288);
	}

	template<typename Type> requires std::floating_point<Type>
	constexpr auto sqrt2() -> Type
	{
		return Type(1.414213562373095048801688724288);
	}
}
