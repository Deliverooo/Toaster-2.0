#include "gl_framebuffer.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

#include "stb/stb_image_write.h"

namespace toaster::gpu
{
	GLFramebuffer::GLFramebuffer(const FramebufferCreateInfo &p_framebuffer_create_info) : m_createInfo(p_framebuffer_create_info)
	{
		for (auto create_info: p_framebuffer_create_info.attachments.attachments)
		{
			if (!isDepthFormat(create_info.format))
				m_colourAttachmentCreateInfos.emplace_back(create_info);
			else
				m_depthStencilAttachmentCreateInfo = create_info;
		}
		recreate();
	}

	GLFramebuffer::~GLFramebuffer()
	{
		gl::deleteFramebuffers(1, &m_framebufferID);
	}

	void GLFramebuffer::bind() const
	{
		gl::bindFramebuffer(gl::FramebufferType::eFramebuffer, m_framebufferID);
	}

	void GLFramebuffer::unbind() const
	{
		gl::bindFramebuffer(gl::FramebufferType::eFramebuffer, 0);
	}

	void GLFramebuffer::resize(uint32 p_width, uint32 p_height)
	{
		if (p_width == 0 || p_height == 0)
		{
			LOG_WARN("Framebuffer dimensions cannot be 0!");
			return;
		}
		m_createInfo.width  = p_width;
		m_createInfo.height = p_height;

		recreate();
	}

	// void GLFramebuffer::saveImageToFile(const io::filesystem::Path &p_path) const
	// {
	// 	uint32                     size = m_createInfo.width * m_createInfo.height * 4;
	// 	std::vector<unsigned char> pixels; // 4 for RGBA
	// 	pixels.resize(size);
	// 	gl::getTextureImage(m_colourAttachmentID, 0, gl::Format::eRGBA, gl::DataType::eUnsignedByte, size, pixels.data());
	//
	// 	stbi_flip_vertically_on_write(true);
	// 	stbi_write_png(p_path.string().c_str(), m_createInfo.width, m_createInfo.height, 4, pixels.data(), m_createInfo.width * 4);
	// }

	uint32 GLFramebuffer::getID() const
	{
		return m_framebufferID;
	}

	uint32 GLFramebuffer::getColourAttachmentID() const
	{
		return m_colourAttachmentIDs[0];
	}

	uint32 GLFramebuffer::getDepthStencilAttachmentID() const
	{
		return m_depthStencilAttachmentID;
	}

	const FramebufferCreateInfo &GLFramebuffer::getCreateInfo() const
	{
		return m_createInfo;
	}

	void GLFramebuffer::recreate()
	{
		if (m_framebufferID != 0)
		{
			gl::deleteTextures(static_cast<gl::SizeI>(m_colourAttachmentIDs.size()), m_colourAttachmentIDs.data());
			gl::deleteTextures(1, &m_depthStencilAttachmentID);
			gl::deleteFramebuffers(1, &m_framebufferID);

			m_colourAttachmentIDs.clear();
			m_depthStencilAttachmentID = 0;
		}

		gl::createFramebuffers(1, &m_framebufferID);

		bool multisample = (m_createInfo.samples > 1);
		if (!m_colourAttachmentCreateInfos.empty())
		{
			m_colourAttachmentIDs.resize(m_colourAttachmentCreateInfos.size());

			gl::createTextures(multisample ? gl::TextureType::e2DMultisample : gl::TextureType::e2D, static_cast<gl::SizeI>(m_colourAttachmentIDs.size()),
							   m_colourAttachmentIDs.data());

			for (uint32 i{0u}; i < m_colourAttachmentIDs.size(); ++i)
			{
				if (multisample)
					gl::textureStorage2DMultisample(m_colourAttachmentIDs[i], static_cast<gl::Int>(m_createInfo.samples),
													getFormat(m_colourAttachmentCreateInfos[i].format), static_cast<gl::Int>(m_createInfo.width),
													static_cast<gl::Int>(m_createInfo.height), true);
				else
					gl::textureStorage2D(m_colourAttachmentIDs[i], 1, getFormat(m_colourAttachmentCreateInfos[i].format),
										 static_cast<gl::Int>(m_createInfo.width), static_cast<gl::Int>(m_createInfo.height));

				gl::textureParameteri(m_colourAttachmentIDs[i], gl::SamplerParameter::eTextureMinFilter, glEnumVal(gl::TextureFiltering::eLinear));
				gl::textureParameteri(m_colourAttachmentIDs[i], gl::SamplerParameter::eTextureMagFilter, glEnumVal(gl::TextureFiltering::eLinear));
				gl::textureParameteri(m_colourAttachmentIDs[i], gl::SamplerParameter::eTextureWrapR, glEnumVal(gl::TextureWrapping::eClampToEdge));
				gl::textureParameteri(m_colourAttachmentIDs[i], gl::SamplerParameter::eTextureWrapS, glEnumVal(gl::TextureWrapping::eClampToEdge));
				gl::textureParameteri(m_colourAttachmentIDs[i], gl::SamplerParameter::eTextureWrapT, glEnumVal(gl::TextureWrapping::eClampToEdge));

				gl::namedFramebufferTexture(m_framebufferID, static_cast<gl::FramebufferAttachment>(static_cast<gl::UInt>(gl::FramebufferAttachment::eColor0) + i),
											m_colourAttachmentIDs[i], 0);
			}
		}
		if (m_depthStencilAttachmentCreateInfo.format != EImageFormat::eInvalid)
		{
			gl::createTextures(multisample ? gl::TextureType::e2DMultisample : gl::TextureType::e2D, 1, &m_depthStencilAttachmentID);

			if (multisample)
				gl::textureStorage2DMultisample(m_depthStencilAttachmentID, static_cast<gl::Int>(m_createInfo.samples),
												getFormat(m_depthStencilAttachmentCreateInfo.format), static_cast<gl::Int>(m_createInfo.width),
												static_cast<gl::Int>(m_createInfo.height), true);
			else
				gl::textureStorage2D(m_depthStencilAttachmentID, 1, getFormat(m_depthStencilAttachmentCreateInfo.format),
									 static_cast<gl::Int>(m_createInfo.width), static_cast<gl::Int>(m_createInfo.height));

			gl::textureParameteri(m_depthStencilAttachmentID, gl::SamplerParameter::eTextureMinFilter, glEnumVal(gl::TextureFiltering::eLinear));
			gl::textureParameteri(m_depthStencilAttachmentID, gl::SamplerParameter::eTextureMagFilter, glEnumVal(gl::TextureFiltering::eLinear));
			gl::textureParameteri(m_depthStencilAttachmentID, gl::SamplerParameter::eTextureWrapR, glEnumVal(gl::TextureWrapping::eClampToEdge));
			gl::textureParameteri(m_depthStencilAttachmentID, gl::SamplerParameter::eTextureWrapS, glEnumVal(gl::TextureWrapping::eClampToEdge));
			gl::textureParameteri(m_depthStencilAttachmentID, gl::SamplerParameter::eTextureWrapT, glEnumVal(gl::TextureWrapping::eClampToEdge));

			gl::namedFramebufferTexture(m_framebufferID, getAttachment(m_depthStencilAttachmentCreateInfo.format), m_depthStencilAttachmentID, 0);
		}

		TST_ASSERT_MSG(gl::checkNamedFramebufferStatus(m_framebufferID, gl::FramebufferType::eFramebuffer) == gl::FramebufferStatus::eComplete,
					   "Incomplete framebuffer!!");
	}

	gl::FramebufferAttachment getAttachment(EImageFormat p_format)
	{
		switch (p_format)
		{
			case EImageFormat::eRed8UN:
			case EImageFormat::eRed8UI:
			case EImageFormat::eRed16UI:
			case EImageFormat::eRed32UI:
			case EImageFormat::eRed32F:
			case EImageFormat::eRG8:
			case EImageFormat::eRG16F:
			case EImageFormat::eRG32F:
			case EImageFormat::eRGB:
			case EImageFormat::eRGBA:
			case EImageFormat::eRGBA16F:
			case EImageFormat::eRGBA32F:
			case EImageFormat::eB10R11G11UF:
			case EImageFormat::eSRGB:
			case EImageFormat::eSRGBA:
				return gl::FramebufferAttachment::eColor0;

			case EImageFormat::eDepth32FStencil8UInt:
			case EImageFormat::eDepth24Stencil8:
				return gl::FramebufferAttachment::eDepthStencil;

			case EImageFormat::eDepth32F: return gl::FramebufferAttachment::eDepth;
		}
		TST_ASSERT(false);
		return static_cast<gl::FramebufferAttachment>(0u);
	}

	gl::Format getFormat(EImageFormat p_format)
	{
		switch (p_format)
		{
			case EImageFormat::eRed8UN: return gl::Format::eRedInteger;
			case EImageFormat::eRed8UI: return gl::Format::eR8UI;
			case EImageFormat::eRed16UI: return gl::Format::eR16UI;
			case EImageFormat::eRed32UI: return gl::Format::eR32UI;
			case EImageFormat::eRed32F: return gl::Format::eR32F;
			case EImageFormat::eRG8: return gl::Format::eRG8;
			case EImageFormat::eRG16F: return gl::Format::eRG16F;
			case EImageFormat::eRG32F: return gl::Format::eRG32F;
			case EImageFormat::eRGB: return gl::Format::eRGB8;
			case EImageFormat::eRGBA: return gl::Format::eRGBA8;
			case EImageFormat::eRGBA16F: return gl::Format::eRGBA16F;
			case EImageFormat::eRGBA32F: return gl::Format::eRGBA32F;
			case EImageFormat::eB10R11G11UF: return gl::Format::eR11FG11FB10F;
			case EImageFormat::eSRGB: return gl::Format::eSRGB8;
			case EImageFormat::eSRGBA: return gl::Format::eSRGB8Alpha8;
			case EImageFormat::eDepth32FStencil8UInt: return gl::Format::eDepth32FStencil8;
			case EImageFormat::eDepth24Stencil8: return gl::Format::eDepth24Stencil8;
			case EImageFormat::eDepth32F: return gl::Format::eDepthComponent32F;
		}
		TST_ASSERT(false);
		return static_cast<gl::Format>(0u);
	}
}
