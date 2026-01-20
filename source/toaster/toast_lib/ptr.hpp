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
}
