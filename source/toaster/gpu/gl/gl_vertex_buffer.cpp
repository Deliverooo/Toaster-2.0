#include "gl_vertex_buffer.hpp"

namespace toaster::gpu
{
	GLVertexBuffer::GLVertexBuffer(uint32 p_size)
	{
		gl::createBuffers(1, &m_vbo);
		gl::bindBuffer(gl::BufferType::eArray, m_vbo);
		gl::bufferData(gl::BufferType::eArray, p_size, nullptr, gl::BufferUsage::eDynamicDraw);
	}

	GLVertexBuffer::GLVertexBuffer(const float *p_data, uint32 p_size)
	{
		gl::createBuffers(1, &m_vbo);
		gl::bindBuffer(gl::BufferType::eArray, m_vbo);
		gl::bufferData(gl::BufferType::eArray, p_size, p_data, gl::BufferUsage::eStaticDraw);
	}

	GLVertexBuffer::~GLVertexBuffer()
	{
		gl::deleteBuffers(1, &m_vbo);
	}

	void GLVertexBuffer::bind()
	{
		gl::bindBuffer(gl::BufferType::eArray, m_vbo);
	}

	void GLVertexBuffer::unbind()
	{
		gl::bindBuffer(gl::BufferType::eArray, 0);
	}

	void GLVertexBuffer::setData(const void *p_data, uint32 p_size)
	{
		gl::bindBuffer(gl::BufferType::eArray, m_vbo);
		gl::bufferSubData(gl::BufferType::eArray, 0, p_size, p_data);
	}

	const VertexBufferLayout &GLVertexBuffer::getLayout()
	{
		return m_layout;
	}

	void GLVertexBuffer::setLayout(const VertexBufferLayout &p_layout)
	{
		m_layout = p_layout;
	}
}
