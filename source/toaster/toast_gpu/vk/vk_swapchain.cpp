#include "vk_swapchain.hpp"

#include "vk_gpu_context.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

namespace toaster::gpu
{
	VKSwapchain::VKSwapchain(VKGPUContext *p_ctx, GLFWwindow *p_window) : m_ctx(p_ctx), m_window(p_window)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");

		_create();
		_createImageViews();
		_createSyncObjects();
		_createCommandBuffers();
		_createDepthResources();
	}

	void VKSwapchain::beginFrame()
	{
		auto &device = m_ctx->getDevice();

		// Wait for the previous frame to be finished before rendering this one
		auto fence_result = device.waitForFences(*m_inFlightFences[m_frameIndex], true, UINT64_MAX);
		if (fence_result != vk::Result::eSuccess)
		{
			TST_ASSERT_MSG(false, "Failed to wait for Fence");
		}
		// Reset the fence so we can signal it later
		device.resetFences(*m_inFlightFences[m_frameIndex]);

		// Signals m_imageAvailableSemaphores[m_frameIndex] when complete
		auto [res, image_index] = m_swapchain.acquireNextImage(UINT64_MAX, *m_imageAvailableSemaphores[m_frameIndex], nullptr);
		m_imageIndex            = image_index;

		if (res == vk::Result::eErrorOutOfDateKHR)
		{
			_recreateSwapchain();
			return;
		}
		if (res != vk::Result::eSuccess)
		{
			TST_ASSERT_MSG(false, "Failed to acquire swapchain image!");
		}

		auto &command_buffer = m_commandBuffers[m_frameIndex];

		command_buffer.reset();

		vk::CommandBufferBeginInfo begin_info{};

		command_buffer.begin(begin_info);

		m_ctx->transitionImageLayout(command_buffer, m_swapchainImages[m_imageIndex], vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
									 vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
									 vk::PipelineStageFlagBits2::eColorAttachmentOutput);

		m_ctx->transitionImageLayout(command_buffer, m_depthImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits2::eNone,
									 vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
									 vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
									 vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests);
	}

	void VKSwapchain::endFrame()
	{
		auto &command_buffer = m_commandBuffers[m_frameIndex];

		m_ctx->transitionImageLayout(command_buffer, m_swapchainImages[m_imageIndex], vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
									 vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eNone, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
									 vk::PipelineStageFlagBits2::eColorAttachmentOutput);

		command_buffer.end();

		auto &graphics_queue = m_ctx->getGraphicsQueue();

		// Waits for the image to be acquired before executing
		// Signals m_renderFinishedSemaphores[m_imageIndex] when finished.
		vk::PipelineStageFlags wait_dst_stage_mask{vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::SubmitInfo         submit_info{};
		submit_info.commandBufferCount   = 1;
		submit_info.pCommandBuffers      = &*m_commandBuffers[m_frameIndex];
		submit_info.pWaitDstStageMask    = &wait_dst_stage_mask;
		submit_info.pWaitSemaphores      = &*m_imageAvailableSemaphores[m_frameIndex];
		submit_info.waitSemaphoreCount   = 1;
		submit_info.pSignalSemaphores    = &*m_renderFinishedSemaphores[m_frameIndex];
		submit_info.signalSemaphoreCount = 1;

		// When we submit the work to the GPU we signal a fence then wait on it before beginning the next frame
		graphics_queue.submit(submit_info, m_inFlightFences[m_frameIndex]);

		// Waits for m_renderFinishedSemaphores[m_imageIndex] to be signalled and submits the work to the GPU.
		vk::PresentInfoKHR present_info{};
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = &*m_renderFinishedSemaphores[m_frameIndex];
		present_info.swapchainCount     = 1;
		present_info.pSwapchains        = &*m_swapchain;
		present_info.pImageIndices      = &m_imageIndex;

		// For some reason, Vulkan-hpp classifies vk::Result::eErrorOutOfDateKHR as an error and automatically throws an exception
		// So this is what I came up with to bypass that :)
		vk::Result res = static_cast<vk::Result>(graphics_queue.getDispatcher()->vkQueuePresentKHR(static_cast<VkQueue>(*graphics_queue),
																								   reinterpret_cast<const VkPresentInfoKHR *>(&present_info)));

		if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || m_framebufferResized)
		{
			m_framebufferResized = false;
			_recreateSwapchain();
		}
		else if (res != vk::Result::eSuccess)
		{
			LOG_ERROR("Failed to present swapchain image!");
			TST_ASSERT(false);
		}

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

	vk::raii::Image &VKSwapchain::getDepthImage()
	{
		return m_depthImage;
	}

	vk::raii::ImageView &VKSwapchain::getDepthImageView()
	{
		return m_depthImageView;
	}

	vk::raii::DeviceMemory &VKSwapchain::getDepthImageMemory()
	{
		return m_depthImageMemory;
	}

	vk::raii::CommandBuffer &VKSwapchain::getCommandBuffer(uint32 p_frame_index)
	{
		return m_commandBuffers[p_frame_index];
	}

	vk::raii::CommandBuffer &VKSwapchain::getCurrentCommandBuffer()
	{
		return m_commandBuffers[m_frameIndex];
	}

	vk::Extent2D VKSwapchain::getExtent() const
	{
		return m_swapchainExtent;
	}

	vk::SurfaceFormatKHR VKSwapchain::getSurfaceFormat() const
	{
		return m_swapchainSurfaceFormat;
	}

	vk::Format VKSwapchain::getDepthFormat() const
	{
		return m_ctx->findDepthFormat();
	}

	uint32 VKSwapchain::getMinImageCount() const
	{
		return m_minImageCount;
	}

	uint32 VKSwapchain::getImageCount() const
	{
		return m_swapchainImages.size();
	}

	void VKSwapchain::setFramebufferResized(bool p_resized)
	{
		m_framebufferResized = p_resized;
	}

	void VKSwapchain::_createImageViews()
	{
		for (auto &img: m_swapchainImages)
			m_swapchainImageViews.emplace_back(m_ctx->createImageView(img, m_swapchainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor));
	}

	void VKSwapchain::_createDepthResources()
	{
		vk::Format depth_format = m_ctx->findDepthFormat();
		m_ctx->createImage(m_swapchainExtent.width, m_swapchainExtent.height, depth_format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
						   vk::MemoryPropertyFlagBits::eDeviceLocal, m_depthImage, m_depthImageMemory);
		m_depthImageView = m_ctx->createImageView(m_depthImage, depth_format, vk::ImageAspectFlagBits::eDepth);
	}

	void VKSwapchain::_createSyncObjects()
	{
		auto &device = m_ctx->getDevice();
		for (uint32 i{0u}; i < VKGPUContext::c_maxFramesInFlight; ++i)
		{
			// I don't know why vk::SemaphoreCreateInfo exists, there are no parameters that you can set for it
			vk::SemaphoreCreateInfo semaphore_create_info{};
			m_imageAvailableSemaphores.emplace_back(device, semaphore_create_info);
			m_renderFinishedSemaphores.emplace_back(device, semaphore_create_info);

			vk::FenceCreateInfo fence_create_info{};
			fence_create_info.flags = vk::FenceCreateFlagBits::eSignaled;
			m_inFlightFences.emplace_back(device, fence_create_info);
		}
	}

	void VKSwapchain::_createCommandBuffers()
	{
		auto &device = m_ctx->getDevice();

		vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.commandBufferCount = VKGPUContext::c_maxFramesInFlight;
		command_buffer_allocate_info.commandPool        = m_ctx->getGraphicsCommandPool();
		command_buffer_allocate_info.level              = vk::CommandBufferLevel::ePrimary;

		m_commandBuffers = vk::raii::CommandBuffers{device, command_buffer_allocate_info};
	}

	void VKSwapchain::_create()
	{
		auto &                     physical_device = m_ctx->getPhysicalDevice();
		auto &                     surface         = m_ctx->getSurface();
		vk::SurfaceCapabilitiesKHR surface_caps    = physical_device.getSurfaceCapabilitiesKHR(surface);

		auto available_surface_formats = physical_device.getSurfaceFormatsKHR(surface);
		auto available_present_modes   = physical_device.getSurfacePresentModesKHR(surface);

		m_swapchainSurfaceFormat = _chooseSwapchainSurfaceFormat(available_surface_formats);
		m_swapchainExtent        = _chooseSwapchainExtent(surface_caps);

		m_minImageCount = _chooseSwapchainMinImageCount(surface_caps);

		vk::SwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.surface          = surface;
		swapchain_create_info.minImageCount    = m_minImageCount;
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

		if (*m_swapchain)
		{
			// I think this should work, but I am not totally sure
			swapchain_create_info.oldSwapchain = std::move(*m_swapchain);
		}

		m_swapchain       = {m_ctx->getDevice(), swapchain_create_info};
		m_swapchainImages = m_swapchain.getImages();
	}

	void VKSwapchain::_recreateSwapchain()
	{
		// Blocks execution until the window is a valid size (for minimisation)
		int32 width;
		int32 height;
		glfwGetFramebufferSize(m_window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetWindowSize(m_window, &width, &height);
			glfwWaitEvents();
		}

		// Wait for the GPU to finish processing anything before recreating, so nothing that depends on the swapchain becomes invalid
		m_ctx->getDevice().waitIdle();

		m_swapchainImageViews.clear();

		_create();
		_createImageViews();
		_createDepthResources();

		m_ctx->getDevice().waitIdle();
	}

	vk::SurfaceFormatKHR VKSwapchain::_chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_available_formats) const
	{
		TST_ASSERT(!p_available_formats.empty());
		// According to my expert research, the most aesthetically pleasing image format is RGBA in the SRGB colour space.
		// If for some reason, your GPU does not support that, then just fall back to the first available format.
		const auto format_it = std::ranges::find_if(p_available_formats, [](const auto &format)
		{
			return format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		});
		return format_it == p_available_formats.end() ? p_available_formats[0] : *format_it;
	}

	vk::PresentModeKHR VKSwapchain::_chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &p_available_present_modes) const
	{
		// The ideal present mode would be mailbox because it is the fastest.
		// However, not every device supports it. But Fifo is guaranteed to be supported, so that is the fallback option
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
		// If the current extent is UINT32_MAX, it means that we can choose our own custom extent
		if (p_surface_capabilities.currentExtent.width != UINT32_MAX)
			return p_surface_capabilities.currentExtent;

		// That extent will be equal to the back buffer's size
		int32 width;
		int32 height;
		glfwGetFramebufferSize(m_window, &width, &height);

		// But we still have to make sure that we clamp the extent between the min and max.
		// I think this probably has to do with certain displays (Apple Retina) having a very high pixel density (DPI).
		return {
			std::clamp<uint32>(width, p_surface_capabilities.minImageExtent.width, p_surface_capabilities.maxImageExtent.width),
			std::clamp<uint32>(height, p_surface_capabilities.minImageExtent.height, p_surface_capabilities.maxImageExtent.height)
		};
	}

	uint32 VKSwapchain::_chooseSwapchainMinImageCount(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const
	{
		// Ideally, we want the min image count to be at least 3. However, if your GPU is bad, it might not be able to handle that many images.
		// So if 3 is greater than the max image count, we fall back to the max image count as the min image count... I don't know if that made sense...
		uint32 min_image_count = std::max(3u, p_surface_capabilities.minImageCount);

		// Apparently, if the maxImageCount == 0, then there is no maximum (unlimited).
		if ((p_surface_capabilities.maxImageCount > 0) && (p_surface_capabilities.maxImageCount < min_image_count))
		{
			min_image_count = p_surface_capabilities.maxImageCount;
		}
		return min_image_count;
	}
}
