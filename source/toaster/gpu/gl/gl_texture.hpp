#pragma once

#include "texture.hpp"
#include <openglhpp/opengl.hpp>

namespace toaster::gpu
{
	class GLTexture2D : public Texture2D
	{
	public:
		GLTexture2D(const io::filesystem::Path &p_path);
		~GLTexture2D() override;

		void bind(uint32 p_slot) const override;

		uint32 getWidth() const override;
		uint32 getHeight() const override;

	private:
		io::filesystem::Path m_path;

		gl::UInt m_textureId{0u};
	};
}
