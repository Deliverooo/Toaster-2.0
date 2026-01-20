#pragma once

#include "texture.hpp"
#include "io/filesystem.hpp"

#include <openglhpp/opengl.hpp>

namespace toaster::gpu
{
	class GLTexture : Texture
	{
	public:
		GLTexture(const io::filesystem::Path &p_path);
		~GLTexture() override;

		void bind() override;
		void unbind() override;

	private:
		io::filesystem::Path m_path;
		gl::UInt             m_textureId{0u};
	};
}
