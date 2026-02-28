#pragma once

#include <openglhpp/opengl.hpp>

#include "toaster/toast_gpu/vertex_buffer.hpp"

namespace toaster::gpu
{
	class GLVertexBuffer : public VertexBuffer
	{
	public:
		explicit GLVertexBuffer(uint32 p_size);
		GLVertexBuffer(const void *p_data, uint32 p_size);
		~GLVertexBuffer() override;

		void bind() override;
		void unbind() override;

		void setData(const void *p_data, uint32 p_size) override;

		void                      setLayout(const VertexBufferLayout &p_layout) override;
		const VertexBufferLayout &getLayout() override;

	private:
		VertexBufferLayout m_layout;
		gl::UInt           m_vbo{0u};
	};
}
