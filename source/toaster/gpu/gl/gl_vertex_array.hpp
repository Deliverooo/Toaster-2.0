#pragma once

#include <openglhpp/opengl.hpp>

#include "vertex_array.hpp"

namespace toaster::gpu
{
	class GLVertexArray : public VertexArray
	{
	public:
		GLVertexArray();
		~GLVertexArray() override;

		void bind() override;
		void unbind() override;

		void addVertexBuffer(const RefPtr<VertexBuffer> &p_vertex_buffer) override;
		void setIndexBuffer(const RefPtr<IndexBuffer> &p_index_buffer) override;

		[[nodiscard]] const std::vector<RefPtr<VertexBuffer> > &getVertexBuffers() const override;
		[[nodiscard]] const RefPtr<IndexBuffer> &               getIndexBuffer() const override;

	private:
		std::vector<RefPtr<VertexBuffer> > m_vertexBuffers;
		RefPtr<IndexBuffer>                m_indexBuffer;

		gl::UInt m_vao{0u};
		uint32   m_vboIndex{0u};
	};
}
