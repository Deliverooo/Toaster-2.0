/*!
 * @file index_buffer.hpp
 */
#pragma once

#include "toaster/toast_lib/ptr.hpp"
#include "toaster/toast_lib/system_types.h"

namespace toaster::gpu
{
	class IndexBuffer
	{
	public:
		static RefPtr<IndexBuffer> create(uint32 p_count);
		static RefPtr<IndexBuffer> create(uint32 *p_data, uint32 p_count);
		virtual                    ~IndexBuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		[[nodiscard]] virtual uint32 getID() const = 0;

		virtual uint32 getIndexCount() = 0;
	};
}
