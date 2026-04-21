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

	auto VKSwapchain::getContext() const -> VKGPUContext *
	{
		return m_ctx;
	}

	auto VKSwapchain::beginFrame() -> void
	{
		auto &device = m_ctx->getLogicalDevice()->getVulkanLogicalDevice();

		// Wait for the previous frame to be finished before rendering this one
		auto fence_result = device.waitForFences(*m_inFlightFences[m_frameIndex], true, UINT64_MAX);
		if (fence_result != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to wait for Fence");

		m_ctx->setCurrentFrameIndex(m_frameIndex);
		m_ctx->performGarbageCollection();

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
			TST_ASSERT_MSG(false, "Failed to acquire swapchain image!");

		auto &command_buffer = m_commandBuffers[m_frameIndex];

		command_buffer.reset();

		vk::CommandBufferBeginInfo begin_info{};

		command_buffer.begin(begin_info);

		m_ctx->transitionImageLayout(command_buffer, m_swapchainImages[m_imageIndex], vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
									 vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
									 vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::ImageAspectFlagBits::eColor);

		m_ctx->transitionImageLayout(command_buffer, m_depthImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits2::eNone,
									 vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
									 vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
									 vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, vk::ImageAspectFlagBits::eDepth);
	}

	auto VKSwapchain::endFrame() -> void
	{
		auto &command_buffer = m_commandBuffers[m_frameIndex];

		m_ctx->transitionImageLayout(command_buffer, m_swapchainImages[m_imageIndex], vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
									 vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eNone, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
									 vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::ImageAspectFlagBits::eColor);

		command_buffer.end();

		auto &graphics_queue = m_ctx->getLogicalDevice()->getGraphicsQueue();

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
		auto res = static_cast<vk::Result>(graphics_queue.getDispatcher()->vkQueuePresentKHR(static_cast<VkQueue>(*graphics_queue),
																							 reinterpret_cast<const VkPresentInfoKHR *>(&present_info)));

		if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || m_framebufferResized)
		{
			m_framebufferResized = false;
			_recreateSwapchain();
		}
		else if (res != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to present swapchain image!");

		m_frameIndex = (m_frameIndex + 1) % VKGPUContext::c_maxFramesInFlight;
	}

	auto VKSwapchain::getFrameIndex() const -> uint32
	{
		return m_frameIndex;
	}

	auto VKSwapchain::getImageIndex() const -> uint32
	{
		return m_imageIndex;
	}

	auto VKSwapchain::getImage(uint32 p_index) -> vk::Image &
	{
		TST_ASSERT_MSG(p_index < m_swapchainImages.size(), "Out of bounds");
		return m_swapchainImages[p_index];
	}

	auto VKSwapchain::getImageView(uint32 p_index) -> vk::raii::ImageView &
	{
		TST_ASSERT_MSG(p_index < m_swapchainImageViews.size(), "Out of bounds");
		return m_swapchainImageViews[p_index];
	}

	auto VKSwapchain::getCurrentImage() -> vk::Image &
	{
		return m_swapchainImages[m_imageIndex];
	}

	auto VKSwapchain::getCurrentImageView() -> vk::raii::ImageView &
	{
		return m_swapchainImageViews[m_imageIndex];
	}

	auto VKSwapchain::getDepthImage() -> vk::raii::Image &
	{
		return m_depthImage;
	}

	auto VKSwapchain::getDepthImageView() -> vk::raii::ImageView &
	{
		return m_depthImageView;
	}

	auto VKSwapchain::getDepthImageMemory() -> vk::raii::DeviceMemory &
	{
		return m_depthImageMemory;
	}

	auto VKSwapchain::getCommandBuffer(uint32 p_frame_index) -> vk::raii::CommandBuffer &
	{
		return m_commandBuffers[p_frame_index];
	}

	auto VKSwapchain::getCurrentCommandBuffer() -> vk::raii::CommandBuffer &
	{
		return m_commandBuffers[m_frameIndex];
	}

	auto VKSwapchain::getExtent() const -> vk::Extent2D
	{
		return m_swapchainExtent;
	}

	auto VKSwapchain::getSurfaceFormat() const -> vk::SurfaceFormatKHR
	{
		return m_swapchainSurfaceFormat;
	}

	auto VKSwapchain::getDepthFormat() const -> vk::Format
	{
		return m_ctx->getPhysicalDevice()->getDepthFormat();
	}

	auto VKSwapchain::getMinImageCount() const -> uint32
	{
		return m_minImageCount;
	}

	auto VKSwapchain::getImageCount() const -> uint32
	{
		return m_swapchainImages.size();
	}

	auto VKSwapchain::setFramebufferResized(bool p_resized) -> void
	{
		m_framebufferResized = p_resized;
	}

	auto VKSwapchain::addResizeCallback(const ResizeCB &p_resize_cb) -> void
	{
		m_resizeCallbacks.emplace_back(p_resize_cb);
	}

	auto VKSwapchain::_createImageViews() -> void
	{
		for (auto &img: m_swapchainImages)
			m_swapchainImageViews.emplace_back(m_ctx->createImageView(img, m_swapchainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1));
	}

	auto VKSwapchain::_createDepthResources() -> void
	{
		m_ctx->createImage(m_swapchainExtent.width, m_swapchainExtent.height, 1, vk::SampleCountFlagBits::e1, getDepthFormat(), vk::ImageTiling::eOptimal,
						   vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_depthImage, m_depthImageMemory);
		m_depthImageView = m_ctx->createImageView(m_depthImage, getDepthFormat(), vk::ImageAspectFlagBits::eDepth, 1);
	}

	auto VKSwapchain::_createSyncObjects() -> void
	{
		auto &device = m_ctx->getLogicalDevice()->getVulkanLogicalDevice();
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

	auto VKSwapchain::_createCommandBuffers() -> void
	{
		auto &device = m_ctx->getLogicalDevice()->getVulkanLogicalDevice();

		vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.commandBufferCount = VKGPUContext::c_maxFramesInFlight;
		command_buffer_allocate_info.commandPool        = m_ctx->getLogicalDevice()->getGraphicsCommandPool();
		command_buffer_allocate_info.level              = vk::CommandBufferLevel::ePrimary;

		m_commandBuffers = vk::raii::CommandBuffers{device, command_buffer_allocate_info};
	}

	auto VKSwapchain::_create() -> void
	{
		auto &                     physical_device = m_ctx->getPhysicalDevice()->getVulkanPhysicalDevice();
		auto &                     surface         = *m_ctx->getLogicalDevice()->getSpecInfo().surface;
		vk::SurfaceCapabilitiesKHR surface_caps    = physical_device.getSurfaceCapabilitiesKHR(&surface);

		auto available_surface_formats = physical_device.getSurfaceFormatsKHR(&surface);
		auto available_present_modes   = physical_device.getSurfacePresentModesKHR(&surface);

		m_swapchainSurfaceFormat = _chooseSwapchainSurfaceFormat(available_surface_formats);
		m_swapchainExtent        = _chooseSwapchainExtent(surface_caps);

		m_minImageCount = _chooseSwapchainMinImageCount(surface_caps);

		vk::SwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.surface          = &surface;
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
			swapchain_create_info.oldSwapchain = *m_swapchain;
		}

		m_swapchain       = {m_ctx->getLogicalDevice()->getVulkanLogicalDevice(), swapchain_create_info};
		m_swapchainImages = m_swapchain.getImages();
	}

	auto VKSwapchain::_recreateSwapchain() -> void
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
		m_ctx->getLogicalDevice()->getVulkanLogicalDevice().waitIdle();

		m_swapchainImageViews.clear();

		_create();
		_createImageViews();
		_createDepthResources();

		for (auto &callback: m_resizeCallbacks)
			callback(m_swapchainExtent.width, m_swapchainExtent.height);

		m_ctx->getLogicalDevice()->getVulkanLogicalDevice().waitIdle();
	}

	auto VKSwapchain::_chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_available_formats) const -> vk::SurfaceFormatKHR
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

	auto VKSwapchain::_chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &p_available_present_modes) const -> vk::PresentModeKHR
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

	auto VKSwapchain::_chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const -> vk::Extent2D
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

	auto VKSwapchain::_chooseSwapchainMinImageCount(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const -> uint32
	{
		// Ideally, we want the min image count to be at least 3. However, if your GPU is bad, it might not be able to handle that many images.
		// So if 3 is greater than the max image count, we fall back to the max image count as the min image count... I don't know if that made sense...
		uint32 min_image_count = std::max(3u, p_surface_capabilities.minImageCount);

		// Apparently, if the maxImageCount == 0, then there is no maximum (unlimited).
		if ((p_surface_capabilities.maxImageCount > 0) && (p_surface_capabilities.maxImageCount < min_image_count))
			min_image_count = p_surface_capabilities.maxImageCount;
		return min_image_count;
	}
}
