#pragma once

#include <concepts>
#include <type_traits>

namespace toaster
{
	// Returns the address of the parameter if it is not a pointer, else it just returns that pointer
	template<typename Type>
	constexpr auto getAddressIfNotPointer(Type &p_param) -> void*
	{
		if constexpr (std::is_pointer_v<Type>)
		{
			return (void*)p_param;
		}
		else
		{
			return (void*)&p_param;
		}
	}
}
