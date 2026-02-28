#include "framebuffer.hpp"
#include "gpu_context.hpp"
#include "swapchain.hpp"
#include "toast_assert.h"

namespace toaster::gpu
{
	Framebuffer::Framebuffer(GPUContext *p_ctx, const FramebufferSpecInfo &p_spec_info, Swapchain *p_swapchain) : m_ctx(p_ctx), m_swapchain(p_swapchain),
																												  m_specInfo(p_spec_info)
	{
		if (!m_specInfo.deriveFromSwapchain)
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

				for (auto &attachment: p_spec_info.attachments)
				{
				}
			}
		}
		else
		{
			TST_ASSERT_MSG(m_swapchain, "Swapchain cannot be nullptr");
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
		if (m_swapchain && m_specInfo.deriveFromSwapchain)
		{
			return m_swapchain->getCurrentFramebuffer();
		}

		return m_framebuffer;
	}

	const FramebufferSpecInfo &Framebuffer::getSpecInfo() const
	{
		return m_specInfo;
	}

	Swapchain *Framebuffer::getSwapchain() const
	{
		return m_swapchain;
	}
}
