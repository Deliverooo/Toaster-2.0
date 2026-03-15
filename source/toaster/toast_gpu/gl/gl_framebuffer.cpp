#include "gl_framebuffer.hpp"

#include "toaster/toast_lib/logging.hpp"
#include "toaster/toast_lib/toast_assert.h"

#include "stb/stb_image_write.h"

namespace toaster::gpu
{
	GLFramebuffer::GLFramebuffer(const FramebufferCreateInfo &p_framebuffer_create_info) : m_createInfo(p_framebuffer_create_info)
	{
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

	void GLFramebuffer::saveImageToFile(const io::filesystem::Path &p_path) const
	{
		uint32                     size = m_createInfo.width * m_createInfo.height * 4;
		std::vector<unsigned char> pixels; // 4 for RGBA
		pixels.resize(size);
		gl::getTextureImage(m_colourAttachmentID, 0, gl::Format::eRGBA, gl::DataType::eUnsignedByte, size, pixels.data());

		stbi_flip_vertically_on_write(true);
		stbi_write_png(p_path.string().c_str(), m_createInfo.width, m_createInfo.height, 4, pixels.data(), m_createInfo.width * 4);
	}

	uint32 GLFramebuffer::getID() const
	{
		return m_framebufferID;
	}

	uint32 GLFramebuffer::getColourAttachmentID() const
	{
		return m_colourAttachmentID;
	}

	uint32 GLFramebuffer::getDepthStencilAttachmentID() const
	{
		return m_depthStencilAttachmentID;
	}

	void GLFramebuffer::recreate()
	{
		if (m_framebufferID != 0)
		{
			gl::deleteTextures(1, &m_colourAttachmentID);
			gl::deleteTextures(1, &m_depthStencilAttachmentID);
			gl::deleteFramebuffers(1, &m_framebufferID);
		}

		gl::createFramebuffers(1, &m_framebufferID);

		gl::createTextures(gl::TextureType::e2D, 1, &m_colourAttachmentID);
		gl::textureStorage2D(m_colourAttachmentID, 1, gl::Format::eRGBA8, static_cast<gl::Int>(m_createInfo.width), static_cast<gl::Int>(m_createInfo.height));

		gl::textureParameteri(m_colourAttachmentID, gl::SamplerParameter::eTextureMinFilter, glEnumVal(gl::TextureFiltering::eLinear));
		gl::textureParameteri(m_colourAttachmentID, gl::SamplerParameter::eTextureMagFilter, glEnumVal(gl::TextureFiltering::eLinear));

		gl::namedFramebufferTexture(m_framebufferID, gl::FramebufferAttachment::eColor0, m_colourAttachmentID, 0);

		gl::createTextures(gl::TextureType::e2D, 1, &m_depthStencilAttachmentID);
		gl::textureStorage2D(m_depthStencilAttachmentID, 1, gl::Format::eDepth24Stencil8, static_cast<gl::Int>(m_createInfo.width),
							 static_cast<gl::Int>(m_createInfo.height));

		gl::namedFramebufferTexture(m_framebufferID, gl::FramebufferAttachment::eDepthStencil, m_depthStencilAttachmentID, 0);

		TST_ASSERT_MSG(gl::checkNamedFramebufferStatus(m_framebufferID, gl::FramebufferType::eFramebuffer) == gl::FramebufferStatus::eComplete,
					   "Incomplete framebuffer!!");
	}

	const FramebufferCreateInfo &GLFramebuffer::getCreateInfo() const
	{
		return m_createInfo;
	}
}
