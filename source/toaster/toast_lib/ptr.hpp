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

	template<typename Type, typename... TArgs>
	UniquePtr<Type> make_unique(TArgs &&... p_args)
	{
		return std::make_unique<Type>(std::forward<TArgs>(p_args)...);
	}

	template<typename Type, typename Type2>
	RefPtr<Type> dynamic_ref_cast(const RefPtr<Type2> &p_other)
	{
		return std::dynamic_pointer_cast<Type>(p_other);
	}
}
