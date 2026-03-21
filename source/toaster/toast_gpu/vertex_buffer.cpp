#include "vertex_buffer.hpp"
#include "gl/gl_vertex_buffer.hpp"

namespace toaster::gpu
{
	RefPtr<IVertexBuffer> IVertexBuffer::create(uint32 p_size)
	{
		return std::make_shared<GLVertexBuffer>(p_size);
	}

	RefPtr<IVertexBuffer> IVertexBuffer::create(void *p_data, uint32 p_size)
	{
		return std::make_shared<GLVertexBuffer>(p_data, p_size);
	}
}
