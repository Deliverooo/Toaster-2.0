#include "gl/gl_texture.hpp"

#include <stb/stb_image.h>

namespace toaster::gpu
{
	GLTexture2D::GLTexture2D(const io::filesystem::Path &p_path) : m_path(p_path)
	{
		gl::createTextures(gl::TextureType::e2D, 1, &m_textureId);

		int32  width, height, num_channels;
		uint8 *data = stbi_load(p_path.string().c_str(), &width, &height, &num_channels, 0);
		if (data)
		{
			gl::bindTexture(gl::TextureType::e2D, m_textureId);

			gl::texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			gl::generateMipmap(GL_TEXTURE_2D);

			gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		stbi_image_free(data);
	}

	GLTexture2D::~GLTexture2D()
	{
		gl::deleteTextures(1, &m_textureId);
	}

	void GLTexture2D::bind(uint32 p_slot) const
	{
		gl::bindTexture(gl::TextureType::e2D, m_textureId);
	}

	uint32 GLTexture2D::getWidth() const
	{
		return 0;
	}

	uint32 GLTexture2D::getHeight() const
	{
		return 0;
	}
}
