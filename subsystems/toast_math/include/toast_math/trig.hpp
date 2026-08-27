#pragma once

namespace tsm
{
	template<typename Type>
	constexpr auto radians(Type p_degrees) -> Type
	{
		return p_degrees * static_cast<Type>(0.01745329251994329576923690768489);
	}

	template<typename Type>
	constexpr auto degrees(Type p_radians) -> Type
	{
		return p_radians * static_cast<Type>(57.295779513082320876798154814105);
	}
}
