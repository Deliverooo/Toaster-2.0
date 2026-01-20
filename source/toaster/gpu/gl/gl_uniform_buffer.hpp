#pragma once

#include <openglhpp/opengl.hpp>

#include "uniform_buffer.hpp"

namespace toaster::gpu
{
	class GLUniformBuffer : public UniformBuffer
	{
	public:
		GLUniformBuffer(uint32 p_size);
		~GLUniformBuffer() override;

		void bind() override;
		void unbind() override;

		void setData(Buffer p_buffer, uint64 p_offset) override;
		void setData(const void *p_data, uint64 p_size, uint64 p_offset) override;

	private:
		gl::UInt m_ubo{0u};
	};
}
