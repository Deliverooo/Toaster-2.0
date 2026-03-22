/*!
 * @file index_buffer.cpp
 */

#include "index_buffer.hpp"
#include "gl/gl_index_buffer.hpp"

namespace toaster::gpu
{
	RefPtr<IIndexBuffer> IIndexBuffer::create(uint32 p_count)
	{
		return make_reference<GLIndexBuffer>(p_count);
	}

	RefPtr<IIndexBuffer> IIndexBuffer::create(uint32 *p_data, uint32 p_count)
	{
		return make_reference<GLIndexBuffer>(p_data, p_count);
	}
}
