/*!
 * @file index_buffer.hpp
 */
#pragma once

#include "toast_lib/ptr.hpp"
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	class IIndexBuffer
	{
	public:
		static RefPtr<IIndexBuffer> create(uint32 p_count);
		static RefPtr<IIndexBuffer> create(uint32 *p_data, uint32 p_count);
		virtual                     ~IIndexBuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		[[nodiscard]] virtual uint32 getID() const = 0;

		[[nodiscard]] virtual uint32 getIndexCount() const = 0;
	};
}
