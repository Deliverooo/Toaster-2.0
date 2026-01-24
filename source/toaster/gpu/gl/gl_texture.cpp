#include "gl/gl_texture.hpp"

#include <stb/stb_image.h>

namespace toaster::gpu
{
	GLTexture2D::GLTexture2D(const io::filesystem::Path &p_path) : m_path(p_path)
	{
		gl::createTextures(gl::TextureType::e2D, &m_textureId);

		int32  width, height, num_channels;
		uint8 *data = stbi_load(p_path.string().c_str(), &width, &height, &num_channels, 0);
		if (data)
		{
			gl::bindTexture(gl::TextureType::e2D, m_textureId);
		}
	}

	GLTexture2D::~GLTexture()
	{
	}

	void GLTexture2D::bind(uint32 p_slot) const
	{
		gl::bindTextureUnit(p_slot, m_textureId);
	}
}
