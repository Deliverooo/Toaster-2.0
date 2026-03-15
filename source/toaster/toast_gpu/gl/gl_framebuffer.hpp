#pragma once

#include <openglhpp/opengl.hpp>
#include "toaster/toast_gpu/framebuffer.hpp"

namespace toaster::gpu
{
	class GLFramebuffer : public Framebuffer
	{
	public:
		GLFramebuffer(const FramebufferCreateInfo &p_framebuffer_create_info);
		~GLFramebuffer() override;

		void bind() const override;
		void unbind() const override;

		void resize(uint32 p_width, uint32 p_height) override;

		void saveImageToFile(const io::filesystem::Path &p_path) const override;

		[[nodiscard]] uint32 getID() const override;
		[[nodiscard]] uint32 getColourAttachmentID() const override;
		[[nodiscard]] uint32 getDepthStencilAttachmentID() const override;

		[[nodiscard]] const FramebufferCreateInfo &getCreateInfo() const override;

		void recreate();

	private:
		FramebufferCreateInfo m_createInfo;

		gl::ID m_framebufferID{0u};

		gl::ID m_colourAttachmentID{0u};
		gl::ID m_depthStencilAttachmentID{0u};
	};
}
