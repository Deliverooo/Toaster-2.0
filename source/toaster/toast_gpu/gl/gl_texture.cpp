#include "gl_texture.hpp"

#include <stb/stb_image.h>

namespace toaster::gpu
{
	GLTexture2D::GLTexture2D(uint32 p_width, uint32 p_height) : m_width(p_width), m_height(p_height)
	{
		m_internalFormat = gl::Format::eRGBA8;
		m_dataFormat     = gl::Format::eRGBA;

		gl::createTextures(gl::TextureType::e2D, 1, &m_textureId);
		gl::textureStorage2D(m_textureId, 1, m_internalFormat, static_cast<gl::Int>(m_width), static_cast<gl::Int>(m_height));

		gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureWrapS, glEnumVal(gl::TextureWrapping::eRepeat));
		gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureWrapT, glEnumVal(gl::TextureWrapping::eRepeat));
		gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureMinFilter, glEnumVal(gl::TextureFiltering::eLinear));
		gl::texParameteri(gl::TextureType::e2D, gl::SamplerParameter::eTextureMagFilter, glEnumVal(gl::TextureFiltering::eNearest));
	}

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

			m_internalFormat = internal_format;
			m_dataFormat     = data_format;

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

			m_internalFormat = gl::Format::eRGBA;
			m_dataFormat     = gl::Format::eRGBA;

			gl::bindTexture(gl::TextureType::e2D, m_textureId);

			constexpr uint32 fallback_data = 0xffff00ff; // magenta

			gl::texImage2D(gl::TextureType::e2D, 0, m_internalFormat, 1, 1, 0, m_dataFormat, gl::DataType::eUnsignedByte, &fallback_data);

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

	void GLTexture2D::setData(void *p_data, uint32 p_size)
	{
		gl::bindTexture(gl::TextureType::e2D, m_textureId);
		gl::textureSubImage2D(m_textureId, 0, 0, 0, static_cast<gl::Int>(m_width), static_cast<gl::Int>(m_height), m_dataFormat, gl::DataType::eUnsignedByte, p_data);
	}

	void GLTexture2D::bind(uint32 p_slot) const
	{
		gl::bindTextureUnit(p_slot, m_textureId);
	}

	uint32 GLTexture2D::getID() const
	{
		return m_textureId;
	}

	uint32 GLTexture2D::getWidth() const
	{
		return 0;
	}

	uint32 GLTexture2D::getHeight() const
	{
		return 0;
	}

	bool GLTexture2D::operator==(const Texture &p_other) const
	{
		return m_textureId == p_other.getID();
	}
}
