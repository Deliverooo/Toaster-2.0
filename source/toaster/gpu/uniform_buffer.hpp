#pragma once

#include "buffer.hpp"
#include "ptr.hpp"

namespace toaster::gpu
{
	class UniformBuffer
	{
	public:
		RefPtr<UniformBuffer> create(uint64 p_size);
		virtual               ~UniformBuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		virtual void setData(Buffer p_buffer, uint64 p_offset = 0u) = 0;
		virtual void setData(const void *p_data, uint64 p_size, uint64 p_offset = 0u) = 0;
	};
}
