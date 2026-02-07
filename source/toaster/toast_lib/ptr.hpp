#pragma once

#include <memory>

namespace toaster
{
	template<typename Type>
	using RefPtr = std::shared_ptr<Type>;

	template<typename Type>
	using WeakRef = std::weak_ptr<Type>;

	template<typename Type>
	using UniquePtr = std::unique_ptr<Type>;

	template<typename Type, typename... TArgs>
	RefPtr<Type> make_reference(TArgs &&... p_args)
	{
		return std::make_shared<Type>(std::forward<TArgs>(p_args)...);
	}
}
