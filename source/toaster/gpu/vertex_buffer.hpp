#pragma once

#include "ptr.hpp"
#include "system_types.h"
#include "vertex_buffer_layout.hpp"

namespace toaster::gpu
{
	class VertexBuffer
	{
	public:
		static RefPtr<VertexBuffer> create(uint32 p_size);
		static RefPtr<VertexBuffer> create(void *p_data, uint32 p_size);
		virtual                     ~VertexBuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		virtual void setData(const void *p_data, uint32 p_size) = 0;

		virtual void                      setLayout(const VertexBufferLayout &p_layout) = 0;
		virtual const VertexBufferLayout &getLayout() = 0;
	};
}
