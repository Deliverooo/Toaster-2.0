#pragma once

#include <openglhpp/opengl.hpp>

#include "toaster/toast_gpu/texture.hpp"

namespace toaster::gpu
{
	class GLTexture2D : public Texture2D
	{
	public:
		GLTexture2D(const io::filesystem::Path &p_path);
		~GLTexture2D() override;

		void bind(uint32 p_slot) const override;

		[[nodiscard]] uint32 getWidth() const override;
		[[nodiscard]] uint32 getHeight() const override;

	private:
		io::filesystem::Path m_path;

		uint32 m_width{0u};
		uint32 m_height{0u};

		gl::UInt m_textureId{0u};
	};
}
