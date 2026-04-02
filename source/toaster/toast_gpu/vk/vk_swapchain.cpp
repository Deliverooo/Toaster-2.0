#include "vk_swapchain.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

namespace toaster::gpu
{
	VKSwapchain::VKSwapchain(VKGPUContext *p_ctx, GLFWwindow *p_window) : m_ctx(p_ctx), m_window(p_window)
	{
		_createSwapchain();
		_createImageViews();
		_createSyncObjects();
	}

	VKSwapchain::~VKSwapchain()
	{
	}

	void VKSwapchain::beginFrame()
	{
		auto &device = m_ctx->getDevice();

		auto fence_result = device.waitForFences(*m_inFlightFences[m_frameIndex], true, UINT64_MAX);
		if (fence_result != vk::Result::eSuccess)
		{
			LOG_ERROR("Failed to wait for Fence");
			TST_ASSERT(false);
		}
		device.resetFences(*m_inFlightFences[m_frameIndex]);

		auto [res, image_index] = m_swapchain.acquireNextImage(UINT64_MAX, *m_imageAvailableSemaphores[m_frameIndex], nullptr);
		m_imageIndex            = image_index;

		if (res == vk::Result::eErrorOutOfDateKHR)
		{
			// _recreateSwapchain();
			return;
		}

		if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR)
		{
			LOG_ERROR("Failed to acquire swapchain image");
			TST_ASSERT(false);
		}

		auto &command_buffer = m_ctx->getCommandBuffer(m_frameIndex);

		command_buffer.reset();

		vk::CommandBufferBeginInfo begin_info{};

		command_buffer.begin(begin_info);

		m_ctx->transitionImageLayout(m_swapchainImages[m_imageIndex], m_frameIndex, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
									 vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
									 vk::PipelineStageFlagBits2::eColorAttachmentOutput);
	}

	void VKSwapchain::endFrame()
	{
		auto &graphics_queue = m_ctx->getGraphicsQueue();

		vk::PipelineStageFlags wait_dst_stage_mask{vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::SubmitInfo         submit_info{};
		submit_info.commandBufferCount   = 1;
		submit_info.pCommandBuffers      = &*m_ctx->getCommandBuffer(m_frameIndex);
		submit_info.pWaitDstStageMask    = &wait_dst_stage_mask;
		submit_info.pWaitSemaphores      = &*m_imageAvailableSemaphores[m_frameIndex];
		submit_info.waitSemaphoreCount   = 1;
		submit_info.pSignalSemaphores    = &*m_renderFinishedSemaphores[m_imageIndex];
		submit_info.signalSemaphoreCount = 1;

		graphics_queue.submit(submit_info, m_inFlightFences[m_frameIndex]);

		vk::PresentInfoKHR present_info{};
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = &*m_renderFinishedSemaphores[m_imageIndex];
		present_info.swapchainCount     = 1;
		present_info.pSwapchains        = &*m_swapchain;
		present_info.pImageIndices      = &m_imageIndex;

		vk::Result res = graphics_queue.presentKHR(present_info);
		if ((res == vk::Result::eSuboptimalKHR) || (res == vk::Result::eErrorOutOfDateKHR))
		{
			// m_framebufferResized = false;
			// _recreateSwapchain();
		}
		else
			TST_ASSERT(res == vk::Result::eSuccess);

		m_frameIndex = (m_frameIndex + 1) % VKGPUContext::c_maxFramesInFlight;
	}

	uint32 VKSwapchain::getFrameIndex() const
	{
		return m_frameIndex;
	}

	uint32 VKSwapchain::getImageIndex() const
	{
		return m_imageIndex;
	}

	vk::Image &VKSwapchain::getImage(uint32 p_index)
	{
		return m_swapchainImages[p_index];
	}

	vk::raii::ImageView &VKSwapchain::getImageView(uint32 p_index)
	{
		return m_swapchainImageViews[p_index];
	}

	vk::Extent2D VKSwapchain::getExtent() const
	{
		return m_swapchainExtent;
	}

	void VKSwapchain::_createSwapchain()
	{
		auto &                     physical_device = m_ctx->getPhysicalDevice();
		auto &                     surface         = m_ctx->getSurface();
		vk::SurfaceCapabilitiesKHR surface_caps    = physical_device.getSurfaceCapabilitiesKHR(surface);

		auto available_surface_formats = physical_device.getSurfaceFormatsKHR(surface);
		auto available_present_modes   = physical_device.getSurfacePresentModesKHR(surface);

		m_swapchainSurfaceFormat = _chooseSwapchainSurfaceFormat(available_surface_formats);
		m_swapchainExtent        = _chooseSwapchainExtent(surface_caps);

		uint32 min_image_count = _chooseSwapchainMinImageCount(surface_caps);

		vk::SwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.surface          = surface;
		swapchain_create_info.minImageCount    = min_image_count;
		swapchain_create_info.imageFormat      = m_swapchainSurfaceFormat.format;
		swapchain_create_info.imageColorSpace  = m_swapchainSurfaceFormat.colorSpace;
		swapchain_create_info.imageExtent      = m_swapchainExtent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
		swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
		swapchain_create_info.preTransform     = surface_caps.currentTransform;
		swapchain_create_info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchain_create_info.presentMode      = _chooseSwapchainPresentMode(available_present_modes);
		swapchain_create_info.clipped          = true;

		m_swapchain       = {m_ctx->getDevice(), swapchain_create_info};
		m_swapchainImages = m_swapchain.getImages();
	}

	void VKSwapchain::_createImageViews()
	{
		vk::ImageViewCreateInfo image_view_create_info{};
		image_view_create_info.viewType         = vk::ImageViewType::e2D;
		image_view_create_info.format           = m_swapchainSurfaceFormat.format;
		image_view_create_info.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
		image_view_create_info.components       = {
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		};
		for (auto &image: m_swapchainImages)
		{
			image_view_create_info.image = image;
			m_swapchainImageViews.emplace_back(m_ctx->getDevice(), image_view_create_info);
		}
	}

	void VKSwapchain::_createSyncObjects()
	{
		auto &device = m_ctx->getDevice();
		for (uint32 i{0u}; i < m_swapchainImageViews.size(); ++i)
			m_renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo{});

		for (uint32 i{0u}; i < VKGPUContext::c_maxFramesInFlight; ++i)
		{
			m_imageAvailableSemaphores.emplace_back(device, vk::SemaphoreCreateInfo{});

			vk::FenceCreateInfo fence_create_info{};
			fence_create_info.flags = vk::FenceCreateFlagBits::eSignaled;
			m_inFlightFences.emplace_back(device, fence_create_info);
		}
	}

	vk::SurfaceFormatKHR VKSwapchain::_chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_available_formats) const
	{
		TST_ASSERT(!p_available_formats.empty());
		const auto format_it = std::ranges::find_if(p_available_formats, [](const auto &format)
		{
			return format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		});
		return format_it == p_available_formats.end() ? p_available_formats[0] : *format_it;
	}

	vk::PresentModeKHR VKSwapchain::_chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &p_available_present_modes) const
	{
		TST_ASSERT(!p_available_present_modes.empty());
		return std::ranges::any_of(p_available_present_modes, [](const auto &present_mode)
		{
			return present_mode == vk::PresentModeKHR::eMailbox;
		})
				   ? vk::PresentModeKHR::eMailbox
				   : vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D VKSwapchain::_chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const
	{
		if (p_surface_capabilities.currentExtent.width != UINT32_MAX)
			return p_surface_capabilities.currentExtent;

		int32 width;
		int32 height;
		glfwGetFramebufferSize(m_window, &width, &height);

		return {
			std::clamp<uint32>(width, p_surface_capabilities.minImageExtent.width, p_surface_capabilities.maxImageExtent.width),
			std::clamp<uint32>(height, p_surface_capabilities.minImageExtent.height, p_surface_capabilities.maxImageExtent.height)
		};
	}

	uint32 VKSwapchain::_chooseSwapchainMinImageCount(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const
	{
		uint32 min_image_count = std::max(3u, p_surface_capabilities.minImageCount);
		if ((p_surface_capabilities.maxImageCount > 0) && (p_surface_capabilities.maxImageCount < min_image_count))
		{
			min_image_count = p_surface_capabilities.maxImageCount;
		}
		return min_image_count;
	}
}
