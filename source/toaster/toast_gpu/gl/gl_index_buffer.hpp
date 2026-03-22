#pragma once

#include <openglhpp/opengl.hpp>

#include "toaster/toast_gpu/index_buffer.hpp"

namespace toaster::gpu
{
	class GLIndexBuffer final : public IIndexBuffer
	{
	public:
		GLIndexBuffer(uint32 p_count);
		GLIndexBuffer(const uint32 *p_data, uint32 p_count);
		~GLIndexBuffer() override;

		void bind() override;
		void unbind() override;

		[[nodiscard]] uint32 getID() const override;

		[[nodiscard]] uint32 getIndexCount() const override;

	private:
		gl::ID m_ebo{0u};
		uint32 m_indexCount{0u};
	};
}
