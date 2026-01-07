#include "framebuffer.hpp"
#include "gpu_context.hpp"
#include "swapchain.hpp"

namespace toaster::gpu
{
	Framebuffer::Framebuffer(GPUContext *p_ctx, const FramebufferSpecInfo &p_spec_info, Swapchain *p_swapchain) : m_gpuContext(p_ctx), m_specInfo(p_spec_info)
	{
		m_width  = p_spec_info.width;
		m_height = p_spec_info.height;

		if (p_spec_info.deriveFromSwapchain && p_swapchain)
		{
			m_framebuffer = p_swapchain->getCurrentFramebuffer();
		}
		else
		{
			uint32 attachment_index{0u};

			for (auto& attachment : p_spec_info.attachments)
			{
			}

		}
	}

	Framebuffer::~Framebuffer()
	{
	}

	uint32 Framebuffer::getWidth() const
	{
		return m_specInfo.width;
	}

	uint32 Framebuffer::getHeight() const
	{
		return m_specInfo.height;
	}

	nvrhi::FramebufferHandle Framebuffer::getHandle() const
	{
		return m_framebuffer;
	}

	const FramebufferSpecInfo &Framebuffer::getSpecInfo() const
	{
		return m_specInfo;
	}
}
