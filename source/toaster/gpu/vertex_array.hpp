#pragma once

#include "index_buffer.hpp"
#include "vertex_buffer.hpp"

namespace toaster::gpu
{
	class VertexArray
	{
	public:
		static RefPtr<VertexArray> create();
		virtual                    ~VertexArray() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		virtual void addVertexBuffer(const RefPtr<VertexBuffer> &p_vertex_buffer) = 0;
		virtual void setIndexBuffer(const RefPtr<IndexBuffer> &p_index_buffer) = 0;

		[[nodiscard]] virtual const std::vector<RefPtr<VertexBuffer> > &getVertexBuffers() const = 0;
		[[nodiscard]] virtual const RefPtr<IndexBuffer> &               getIndexBuffer() const = 0;
	};
}
