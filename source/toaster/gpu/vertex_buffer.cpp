#include "vertex_buffer.hpp"
#include "gl/gl_vertex_buffer.hpp"

namespace toaster::gpu
{
	RefPtr<VertexBuffer> VertexBuffer::create(uint32 p_size)
	{
		return std::make_shared<GLVertexBuffer>(p_size);
	}

	RefPtr<VertexBuffer> VertexBuffer::create(void *p_data, uint32 p_size)
	{
		return std::make_shared<GLVertexBuffer>(p_data, p_size);
	}
}
