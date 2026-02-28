#pragma once

#include <openglhpp/opengl.hpp>

#include "toaster/toast_gpu/index_buffer.hpp"

namespace toaster::gpu
{
	class GLIndexBuffer final : public IndexBuffer
	{
	public:
		GLIndexBuffer(const uint32 *p_data, uint32 p_count);
		~GLIndexBuffer() override;

		void bind() override;
		void unbind() override;

		uint32 getIndexCount() override;

	private:
		gl::UInt m_ebo{0u};
		uint32   m_indexCount{0u};
	};
}
