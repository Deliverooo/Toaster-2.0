#pragma once

#include <openglhpp/opengl.hpp>
#include "../framebuffer.hpp"

namespace toaster::gpu
{
	class GLFramebuffer : public IFramebuffer
	{
	public:
		GLFramebuffer(const FramebufferCreateInfo &p_framebuffer_create_info);
		~GLFramebuffer() override;

		void bind() const override;
		void unbind() const override;

		void resize(uint32 p_width, uint32 p_height) override;

		[[nodiscard]] uint32 getID() const override;
		[[nodiscard]] uint32 getColourAttachmentID() const override;
		[[nodiscard]] uint32 getDepthStencilAttachmentID() const override;

		[[nodiscard]] const FramebufferCreateInfo &getCreateInfo() const override;

		void recreate();

	private:
		FramebufferCreateInfo m_createInfo;

		gl::ID m_framebufferID{0u};

		std::vector<gl::ID>                       m_colourAttachmentIDs{0u};
		std::vector<FramebufferTextureCreateInfo> m_colourAttachmentCreateInfos;

		gl::ID                       m_depthStencilAttachmentID{0u};
		FramebufferTextureCreateInfo m_depthStencilAttachmentCreateInfo{};
	};

	gl::FramebufferAttachment getAttachment(EImageFormat p_format);
	gl::Format getFormat(EImageFormat p_format);
}
