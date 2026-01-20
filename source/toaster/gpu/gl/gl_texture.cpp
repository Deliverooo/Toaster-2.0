#include "gl/gl_texture.hpp"

#include <stb/stb_image.h>

namespace toaster::gpu
{
	GLTexture::GLTexture(const io::filesystem::Path &p_path) : m_path(p_path)
	{
		gl::genTextures(1, &m_textureId);

		int32  width, height, num_channels;
		uint8 *data = stbi_load(p_path.string().c_str(), &width, &height, &num_channels, 0);
		if (data)
		{
			// gl::Format
			gl::bindTexture(gl::TextureType::e2D, m_textureId);
		}
	}

	GLTexture::~GLTexture()
	{
	}

	void GLTexture::bind()
	{
	}

	void GLTexture::unbind()
	{
	}
}
