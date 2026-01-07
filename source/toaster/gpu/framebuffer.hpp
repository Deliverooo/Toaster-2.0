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
		bool deriveFromSwapchain{false};

		// Sets the blend mode for all attachments
		bool blend{true};
	};

	class Framebuffer
	{
	public:
		Framebuffer(GPUContext *p_ctx, const FramebufferSpecInfo &p_spec_info, Swapchain *p_swapchain = nullptr);
		~Framebuffer();

		uint32 getWidth() const;
		uint32 getHeight() const;

		nvrhi::FramebufferHandle   getHandle() const;
		const FramebufferSpecInfo &getSpecInfo() const;

	private:
		GPUContext *m_gpuContext{nullptr};

		FramebufferSpecInfo m_specInfo;
		uint32              m_width{0u};
		uint32              m_height{0u};

		nvrhi::FramebufferHandle m_framebuffer{nullptr};
	};
}
