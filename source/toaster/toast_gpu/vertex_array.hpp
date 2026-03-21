#pragma once

#include "index_buffer.hpp"
#include "vertex_buffer.hpp"

namespace toaster::gpu
{
	class IVertexArray
	{
	public:
		static RefPtr<IVertexArray> create();
		virtual                    ~IVertexArray() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		virtual void addVertexBuffer(const RefPtr<IVertexBuffer> &p_vertex_buffer) = 0;
		virtual void setIndexBuffer(const RefPtr<IIndexBuffer> &p_index_buffer) = 0;

		[[nodiscard]] virtual const std::vector<RefPtr<IVertexBuffer> > &getVertexBuffers() const = 0;
		[[nodiscard]] virtual const RefPtr<IIndexBuffer> &               getIndexBuffer() const = 0;
	};
}
