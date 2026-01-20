#include "uniform_buffer.hpp"
#include "gl/gl_uniform_buffer.hpp"

namespace toaster::gpu
{
	RefPtr<UniformBuffer> UniformBuffer::create(uint64 p_size)
	{
		return std::make_shared<GLUniformBuffer>(p_size);
	}
}
