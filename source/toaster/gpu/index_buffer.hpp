/*!
 * @file index_buffer.hpp
 */
#pragma once

#include "ptr.hpp"
#include "system_types.h"

namespace toaster::gpu
{
	class IndexBuffer
	{
	public:
		static RefPtr<IndexBuffer> create(uint32 *p_data, uint32 p_count);
		virtual                    ~IndexBuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		virtual uint32 getIndexCount() = 0;
	};
}
