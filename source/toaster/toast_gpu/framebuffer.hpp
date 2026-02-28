#pragma once

#include <nvrhi/nvrhi.h>
#include <vulkan/vulkan.hpp>

#include "system_types.h"

namespace toaster::gpu
{
	class GPUContext;
	class Swapchain;

	struct FramebufferAttachment
	{
		nvrhi::Format        format{nvrhi::Format::UNKNOWN};
		bool                 blend{true};
		nvrhi::BlendFactor   blendMode{nvrhi::BlendFactor::OneMinusSrcAlpha};
		vk::AttachmentLoadOp loadOp{vk::AttachmentLoadOp::eDontCare};
	};

	struct FramebufferSpecInfo
	{
		uint32       width{0u};
		uint32       height{0u};
		nvrhi::Color clearColour{1.0f, 0.0f, 1.0f, 1.0f};

		std::vector<FramebufferAttachment> attachments;

		// If true, all framebuffer relevant information will be gathered from the swapchain
		// This essentially means that the framebuffer class will just become a wrapper for the swapchain's current framebuffer
		bool deriveFromSwapchain{false};

		// Sets the blend mode for all attachments
		bool blend{true};
	};

	class Framebuffer
	{
	public:
		Framebuffer(GPUContext *p_ctx, const FramebufferSpecInfo &p_spec_info, Swapchain *p_swapchain = nullptr);
		~Framebuffer();

		[[nodiscard]] uint32 getWidth() const;
		[[nodiscard]] uint32 getHeight() const;

		[[nodiscard]] nvrhi::FramebufferHandle   getHandle() const;
		[[nodiscard]] const FramebufferSpecInfo &getSpecInfo() const;

		Swapchain *getSwapchain() const;

	private:
		GPUContext *m_ctx{nullptr};

		// If deriveFromSwapchain is set to true in FramebufferSpecInfo, this acts as a reference to use when calling getHandle().
		// It will then just retrieve and return the swapchain's current framebuffer.
		Swapchain *m_swapchain{nullptr};

		FramebufferSpecInfo m_specInfo;
		uint32              m_width{0u};
		uint32              m_height{0u};

		nvrhi::FramebufferHandle m_framebuffer{nullptr};
	};
}
