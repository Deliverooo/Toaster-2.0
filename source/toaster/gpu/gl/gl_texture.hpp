#pragma once

#include "texture.hpp"
#include "io/filesystem.hpp"

#include <openglhpp/opengl.hpp>

namespace toaster::gpu
{
	class GLTexture2D : Texture2D
	{
	public:
		GLTexture2D(const io::filesystem::Path &p_path);
		~GLTexture2D() override;

		void bind(uint32 p_slot) const override;

	private:
		io::filesystem::Path m_path;

		gl::UInt m_textureId{0u};
	};
}
