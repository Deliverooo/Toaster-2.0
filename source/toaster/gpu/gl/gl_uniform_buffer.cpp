#include "gl/gl_uniform_buffer.hpp"

namespace toaster::gpu
{
	GLUniformBuffer::GLUniformBuffer(uint32 p_size)
	{
		gl::createBuffers(1, &m_ubo);
		gl::bindBuffer(gl::BufferType::eUniform, m_ubo);
		gl::bufferData(gl::BufferType::eUniform, p_size, nullptr, gl::BufferUsage::eDynamicDraw);
	}

	GLUniformBuffer::~GLUniformBuffer()
	{
		gl::deleteBuffers(1, &m_ubo);
	}

	void GLUniformBuffer::bind()
	{
		gl::bindBuffer(gl::BufferType::eUniform, m_ubo);
	}

	void GLUniformBuffer::unbind()
	{
		gl::bindBuffer(gl::BufferType::eUniform, 0);
	}

	void GLUniformBuffer::setData(Buffer p_buffer, uint64 p_offset)
	{
		gl::bindBuffer(gl::BufferType::eUniform, m_ubo);
		gl::bufferSubData(gl::BufferType::eUniform, p_offset, p_buffer.size(), p_buffer.data());
	}

	void GLUniformBuffer::setData(const void *p_data, uint64 p_size, uint64 p_offset)
	{
		gl::bindBuffer(gl::BufferType::eUniform, m_ubo);
		gl::bufferSubData(gl::BufferType::eUniform, p_offset, p_size, p_data);
	}
}
