#pragma once

#include <openglhpp/opengl.hpp>

#include "../vertex_array.hpp"

namespace toaster::gpu
{
	class GLVertexArray : public IVertexArray
	{
	public:
		GLVertexArray();
		~GLVertexArray() override;

		void bind() override;
		void unbind() override;

		void addVertexBuffer(const RefPtr<IVertexBuffer> &p_vertex_buffer) override;
		void setIndexBuffer(const RefPtr<IIndexBuffer> &p_index_buffer) override;

		[[nodiscard]] const std::vector<RefPtr<IVertexBuffer> > &getVertexBuffers() const override;
		[[nodiscard]] const RefPtr<IIndexBuffer> &               getIndexBuffer() const override;

	private:
		std::vector<RefPtr<IVertexBuffer> > m_vertexBuffers;
		RefPtr<IIndexBuffer>                m_indexBuffer;

		gl::ID m_vao{0u};
		uint32 m_vboIndex{0u};
	};
}
