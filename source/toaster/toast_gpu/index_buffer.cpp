/*!
 * @file index_buffer.cpp
 */

#include "index_buffer.hpp"
#include "gl/gl_index_buffer.hpp"

namespace toaster::gpu
{
	RefPtr<IndexBuffer> IndexBuffer::create(uint32 p_count)
	{
		return std::make_shared<GLIndexBuffer>(p_count);
	}

	RefPtr<IndexBuffer> IndexBuffer::create(uint32 *p_data, uint32 p_count)
	{
		return std::make_shared<GLIndexBuffer>(p_data, p_count);
	}
}
