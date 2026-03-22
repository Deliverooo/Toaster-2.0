#pragma once

#include "toaster/toast_lib/ptr.hpp"
#include "toaster/toast_lib/system_types.h"

#include "vertex_buffer_layout.hpp"

namespace toaster::gpu
{
	class IVertexBuffer
	{
	public:
		/*!
		 * @brief Creates a vertex buffer with the specified size
		 * @details if you are using a vertex struct e.g. MeshVertex, make sure to multiply the size of the vertices by the size of the struct.
		 * @code
		 * auto vb = gpu::VertexBuffer::Create(vertices.size() * sizeof(MeshVertex));
		 * @endcode
		 * @param p_size the number of vertices
		 * @return a RefPtr to the vertex buffer
		 */
		static RefPtr<IVertexBuffer> create(uint32 p_size);
		/*!
		 * @brief Creates a vertex buffer with the specified size and fills it with the provided data
		 * @details if you are using a vertex struct e.g. MeshVertex, make sure to multiply the size of the vertices by the size of the struct.
		 * @code
		 * auto vb = gpu::VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(MeshVertex));
		 * @endcode
		 * @param p_size the number of vertices
		 * @param p_data the vertex data
		 * @return a RefPtr to the vertex buffer
		 */
		static RefPtr<IVertexBuffer> create(void *p_data, uint32 p_size);

		virtual ~IVertexBuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		virtual void setData(const void *p_data, uint32 p_size) = 0;
		virtual void setLayout(const VertexBufferLayout &p_layout) = 0;

		[[nodiscard]] virtual uint32 getID() const = 0;

		virtual const VertexBufferLayout &              getLayout() = 0;
		[[nodiscard]] virtual const VertexBufferLayout &getLayout() const = 0;
	};
}
