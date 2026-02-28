#include "gl_vertex_array.hpp"

#include "toaster/toast_lib/toast_assert.h"

namespace toaster::gpu
{
	static gl::DataType getComponentType(EShaderDataType p_type)
	{
		switch (p_type)
		{
			case EShaderDataType::eFloat:
			case EShaderDataType::eFloat2:
			case EShaderDataType::eFloat3:
			case EShaderDataType::eFloat4:
			case EShaderDataType::eMat3:
			case EShaderDataType::eMat4: { return gl::DataType::eFloat; }
			case EShaderDataType::eInt:
			case EShaderDataType::eInt2:
			case EShaderDataType::eInt3:
			case EShaderDataType::eInt4: { return gl::DataType::eInt; }
			case EShaderDataType::eBool: { return gl::DataType::eBool; }
		}
		return static_cast<gl::DataType>(0);
	}

	GLVertexArray::GLVertexArray()
	{
		gl::createVertexArrays(1, &m_vao);
		gl::bindVertexArray(m_vao);
	}

	GLVertexArray::~GLVertexArray()
	{
		gl::deleteVertexArrays(1, &m_vao);
	}

	void GLVertexArray::bind()
	{
		gl::bindVertexArray(m_vao);
	}

	void GLVertexArray::unbind()
	{
		gl::bindVertexArray(0);
	}

	void GLVertexArray::addVertexBuffer(const RefPtr<VertexBuffer> &p_vertex_buffer)
	{
		TST_ASSERT_MSG(!p_vertex_buffer->getLayout().getElements().empty(), "Vertex Buffer has no layout!");

		gl::bindVertexArray(m_vao);

		p_vertex_buffer->bind();

		const auto &vbl = p_vertex_buffer->getLayout();
		for (const auto &elem: vbl)
		{
			switch (elem.type)
			{
				case EShaderDataType::eFloat:
				case EShaderDataType::eFloat2:
				case EShaderDataType::eFloat3:
				case EShaderDataType::eFloat4:
				{
					gl::enableVertexAttribArray(m_vboIndex);
					gl::vertexAttribPointer(m_vboIndex, static_cast<gl::Int>(elem.getComponentCount()), getComponentType(elem.type), elem.normalized,
											static_cast<gl::SizeI>(vbl.getStride()), (reinterpret_cast<const void *>(elem.offset)));
					m_vboIndex++;
					break;
				}

				case EShaderDataType::eInt:
				case EShaderDataType::eBool:
				case EShaderDataType::eInt2:
				case EShaderDataType::eInt3:
				case EShaderDataType::eInt4:
				{
					gl::enableVertexAttribArray(m_vboIndex);
					gl::vertexAttribIPointer(m_vboIndex, static_cast<gl::Int>(elem.getComponentCount()), getComponentType(elem.type),
											 static_cast<gl::SizeI>(vbl.getStride()), reinterpret_cast<const void *>(elem.offset));
					m_vboIndex++;
					break;
				}
				case EShaderDataType::eMat3:
				case EShaderDataType::eMat4:
				{
					const uint8 count = elem.getComponentCount();
					for (uint8 i = 0u; i < count; i++)
					{
						gl::enableVertexAttribArray(m_vboIndex);
						gl::vertexAttribPointer(m_vboIndex, count, getComponentType(elem.type), elem.normalized, static_cast<gl::SizeI>(vbl.getStride()),
												reinterpret_cast<const void *>(elem.offset + sizeof(float) * count * i));
						gl::vertexAttribDivisor(m_vboIndex, 1);
						m_vboIndex++;
					}
					break;
				}
			}
		}

		m_vertexBuffers.push_back(p_vertex_buffer);
	}

	void GLVertexArray::setIndexBuffer(const RefPtr<IndexBuffer> &p_index_buffer)
	{
		gl::bindVertexArray(m_vao);
		p_index_buffer->bind();
		m_indexBuffer = p_index_buffer;
	}

	const std::vector<RefPtr<VertexBuffer> > &GLVertexArray::getVertexBuffers() const
	{
		return m_vertexBuffers;
	}

	const RefPtr<IndexBuffer> &GLVertexArray::getIndexBuffer() const
	{
		return m_indexBuffer;
	}
}
