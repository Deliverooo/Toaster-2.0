#include "gl_texture.hpp"

#include <stb/stb_image.h>

namespace toaster::gpu
{
	GLTexture2D::GLTexture2D(const io::filesystem::Path &p_path) : m_path(p_path)
	{
		gl::createTextures(gl::TextureType::e2D, 1, &m_textureId);

		stbi_set_flip_vertically_on_load(true);

		int32  width, height, num_channels;
		uint8 *data = stbi_load(p_path.string().c_str(), &width, &height, &num_channels, 0);
		if (data)
		{
			m_width  = width;
			m_height = height;

			gl::bindTexture(gl::TextureType::e2D, m_textureId);

			gl::Format internal_format = gl::Format::eRGBA8;
			gl::Format data_format     = gl::Format::eRGBA;
			if (num_channels == 4)
			{
				internal_format = gl::Format::eRGBA8;
				data_format     = gl::Format::eRGBA;
			}
			else if (num_channels == 3)
			{
				internal_format = gl::Format::eRGB8;
				data_format     = gl::Format::eRGB;
			}

			gl::texImage2D(gl::TextureType::e2D, 0, internal_format, width, height, 0, data_format, gl::DataType::eUnsignedByte, data);

			gl::generateMipmap(gl::TextureType::e2D);

			gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureWrapS, glEnumVal(gl::TextureWrapping::eRepeat));
			gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureWrapT, glEnumVal(gl::TextureWrapping::eRepeat));
			gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureMinFilter, glEnumVal(gl::TextureFiltering::eLinearMipmapLinear));
			gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureMagFilter, glEnumVal(gl::TextureFiltering::eLinear));
		}
		else
		{
			m_width  = 1;
			m_height = 1;

			gl::bindTexture(gl::TextureType::e2D, m_textureId);

			constexpr uint32 fallback_data = 0xffff00ff; // magenta

			gl::texImage2D(gl::TextureType::e2D, 0, gl::Format::eRGBA, 1, 1, 0, gl::Format::eRGBA, gl::DataType::eUnsignedByte, &fallback_data);

			gl::generateMipmap(gl::TextureType::e2D);

			gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureWrapS, glEnumVal(gl::TextureWrapping::eRepeat));
			gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureWrapT, glEnumVal(gl::TextureWrapping::eRepeat));
			gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureMinFilter, glEnumVal(gl::TextureFiltering::eLinearMipmapLinear));
			gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureMagFilter, glEnumVal(gl::TextureFiltering::eLinear));
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
