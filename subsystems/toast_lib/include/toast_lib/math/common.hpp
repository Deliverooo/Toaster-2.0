#pragma once

namespace tsm
{
	template<typename Type>
	constexpr auto mix(Type p_a, Type p_b, Type p_t) -> Type { return p_a + p_t * (p_b - p_a); }
}
