#pragma once
#include "ref_ptr.hpp"

namespace tst
{
	using ResourceDescriptorInfo = void *;

	class RendererResource : public RefCounted
	{
	public:
		virtual ResourceDescriptorInfo getDescriptorInfo() const = 0;
	};
}
