#include "gl_index_buffer.hpp"

namespace toaster::gpu
{
	GLIndexBuffer::GLIndexBuffer(uint32 p_count) : m_indexCount(p_count)
	{
		gl::createBuffers(1, &m_ebo);
		gl::bindBuffer(gl::BufferType::eElementArray, m_ebo);
		gl::bufferData(gl::BufferType::eElementArray, static_cast<gl::SizeIPtr>(p_count * sizeof(uint32)), nullptr, gl::BufferUsage::eDynamicDraw);
	}

	GLIndexBuffer::GLIndexBuffer(const uint32 *p_data, uint32 p_count) : m_indexCount(p_count)
	{
		gl::createBuffers(1, &m_ebo);
		gl::bindBuffer(gl::BufferType::eElementArray, m_ebo);
		gl::bufferData(gl::BufferType::eElementArray, static_cast<gl::SizeIPtr>(p_count * sizeof(uint32)), p_data, gl::BufferUsage::eStaticDraw);
	}

	GLIndexBuffer::~GLIndexBuffer()
	{
		gl::deleteBuffers(1, &m_ebo);
	}

	void GLIndexBuffer::bind()
	{
		gl::bindBuffer(gl::BufferType::eElementArray, m_ebo);
	}

	void GLIndexBuffer::unbind()
	{
		gl::bindBuffer(gl::BufferType::eElementArray, 0);
	}

	uint32 GLIndexBuffer::getID() const
	{
		return m_ebo;
	}

	uint32 GLIndexBuffer::getIndexCount() const
	{
		return m_indexCount;
	}
}
