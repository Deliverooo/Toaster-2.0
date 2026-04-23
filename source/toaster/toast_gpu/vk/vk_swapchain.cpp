#include "vk_swapchain.hpp"

#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKSwapchain::VKSwapchain(VKLogicalDevice *p_dev, vk::SurfaceKHR *p_surface) : m_device(p_dev), m_windowSurface(p_surface),
																				  m_commandBuffers(p_dev, vk::QueueFlagBits::eGraphics,
																								   p_dev->getSpecInfo().maxFramesInFlight, true)
	{
		TST_ASSERT_MSG(p_dev, "Device cannot be null");

		_create();
		_createImageViews();
		_createSyncObjects();
		_createDepthResources();
	}

	auto VKSwapchain::getDevice() const -> VKLogicalDevice *
	{
		return m_device;
	}

	auto VKSwapchain::beginFrame() -> void
	{
		// Wait for the previous frame to be finished before rendering this one
		m_commandBuffers.waitForFence(m_frameIndex);

		if (m_beginFrameCallback)
			m_beginFrameCallback(m_device, m_frameIndex);

		// Reset the fence so we can signal it later
		m_commandBuffers.resetFence(m_frameIndex);

		auto [res, image_index] = m_swapchain.acquireNextImage(UINT64_MAX, *m_imageAvailableSemaphores[m_frameIndex], nullptr);
		m_imageIndex            = image_index;

		if (res == vk::Result::eErrorOutOfDateKHR)
		{
			_recreateSwapchain();
			return;
		}
		if (res != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to acquire swapchain image!");

		m_commandBuffers.resetCommandBuffer(m_frameIndex);
		m_commandBuffers.begin(m_frameIndex);

		m_device->transitionImageLayout(m_commandBuffers.getVulkanCommandBuffer(m_frameIndex), m_swapchainImages[m_imageIndex], vk::ImageLayout::eUndefined,
										vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite,
										vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eColorAttachmentOutput, 1,
										vk::ImageAspectFlagBits::eColor);

		m_device->transitionImageLayout(m_commandBuffers.getVulkanCommandBuffer(m_frameIndex), m_depthImage, vk::ImageLayout::eUndefined,
										vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
										vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
										vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, 1,
										vk::ImageAspectFlagBits::eDepth);
	}

	auto VKSwapchain::endFrame() -> void
	{
		m_device->transitionImageLayout(m_commandBuffers.getVulkanCommandBuffer(m_frameIndex), m_swapchainImages[m_imageIndex], vk::ImageLayout::eColorAttachmentOptimal,
										vk::ImageLayout::ePresentSrcKHR, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eNone,
										vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eColorAttachmentOutput, 1,
										vk::ImageAspectFlagBits::eColor);

		m_commandBuffers.end(m_frameIndex);

		// Waits for the image to be acquired before executing
		// When we submit the work to the GPU we signal a fence then wait on it before beginning the next frame
		m_commandBuffers.submit(m_frameIndex, vk::PipelineStageFlagBits2::eColorAttachmentOutput, {*m_imageAvailableSemaphores[m_frameIndex]},
								{*m_renderFinishedSemaphores[m_frameIndex]});

		vk::PresentInfoKHR present_info{};
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = &*m_renderFinishedSemaphores[m_frameIndex];
		present_info.swapchainCount     = 1;
		present_info.pSwapchains        = &*m_swapchain;
		present_info.pImageIndices      = &m_imageIndex;

		// For some reason, Vulkan-hpp classifies vk::Result::eErrorOutOfDateKHR as an error and automatically throws an exception
		// So this is what I came up with to bypass that :)
		auto res = static_cast<vk::Result>(m_device->getGraphicsQueue().getDispatcher()->vkQueuePresentKHR(static_cast<VkQueue>(*m_device->getGraphicsQueue()),
																										   reinterpret_cast<const VkPresentInfoKHR *>(&present_info)));

		if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || m_framebufferResized)
		{
			m_framebufferResized = false;
			_recreateSwapchain();
		}
		else if (res != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to present swapchain image!");

		m_frameIndex = (m_frameIndex + 1) % m_device->getSpecInfo().maxFramesInFlight;
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
		return m_commandBuffers.getVulkanCommandBuffer(p_frame_index);
	}

	auto VKSwapchain::getCurrentCommandBuffer() -> vk::raii::CommandBuffer &
	{
		return m_commandBuffers.getVulkanCommandBuffer(m_frameIndex);
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
		return m_device->getPhysicalDevice()->getDepthFormat();
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

	auto VKSwapchain::setBeginFrameCallback(const BeginFrameCB &p_begin_frame_cb) -> void
	{
		m_beginFrameCallback = p_begin_frame_cb;
	}

	auto VKSwapchain::setResizeCallback(const ResizeCB &p_resize_cb) -> void
	{
		m_resizeCallback = p_resize_cb;
	}

	auto VKSwapchain::setHandleMinimisationCallback(const HandleMinimisationCB &p_handle_minimisation_callback) -> void
	{
		m_handleMinimisationCallback = p_handle_minimisation_callback;
	}

	auto VKSwapchain::setGetWindowBackBufferSizeCallback(const GetWindowBackBufferSizeCB &p_get_window_back_buffer_size_callback) -> void
	{
		m_getWindowBackBufferSizeCallback = p_get_window_back_buffer_size_callback;
	}

	auto VKSwapchain::_createImageViews() -> void
	{
		for (auto &img: m_swapchainImages)
			m_swapchainImageViews.emplace_back(m_device->createImageView(img, m_swapchainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1));
	}

	auto VKSwapchain::_createDepthResources() -> void
	{
		m_device->createImage(m_swapchainExtent.width, m_swapchainExtent.height, 1, vk::SampleCountFlagBits::e1, getDepthFormat(), vk::ImageTiling::eOptimal,
							  vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_depthImage, m_depthImageMemory);
		m_depthImageView = m_device->createImageView(m_depthImage, getDepthFormat(), vk::ImageAspectFlagBits::eDepth, 1);
	}

	auto VKSwapchain::_createSyncObjects() -> void
	{
		for (uint32 i{0u}; i < m_device->getSpecInfo().maxFramesInFlight; ++i)
		{
			// I don't know why vk::SemaphoreCreateInfo exists, there are no parameters that you can set for it
			vk::SemaphoreCreateInfo semaphore_create_info{};
			m_imageAvailableSemaphores.emplace_back(m_device->getVulkanLogicalDevice(), semaphore_create_info);
			m_renderFinishedSemaphores.emplace_back(m_device->getVulkanLogicalDevice(), semaphore_create_info);
		}
	}

	auto VKSwapchain::_create() -> void
	{
		auto &                     physical_device = m_device->getPhysicalDevice()->getVulkanPhysicalDevice();
		vk::SurfaceCapabilitiesKHR surface_caps    = physical_device.getSurfaceCapabilitiesKHR(*m_windowSurface);

		auto available_surface_formats = physical_device.getSurfaceFormatsKHR(*m_windowSurface);
		auto available_present_modes   = physical_device.getSurfacePresentModesKHR(*m_windowSurface);

		m_swapchainSurfaceFormat = m_device->getPhysicalDevice()->chooseSwapchainSurfaceFormat(*m_windowSurface);

		uint32 width{0u};
		uint32 height{0u};

		// The fallback extent will be equal to the back buffer's size
		if (m_getWindowBackBufferSizeCallback)
		{
			const auto [w, h]{m_getWindowBackBufferSizeCallback()};
			width  = w;
			height = h;
		}
		m_swapchainExtent = m_device->getPhysicalDevice()->chooseSwapchainExtent(*m_windowSurface, width, height);

		m_minImageCount = m_device->getPhysicalDevice()->chooseSwapchainMinImageCount(*m_windowSurface);

		vk::SwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.surface          = *m_windowSurface;
		swapchain_create_info.minImageCount    = m_minImageCount;
		swapchain_create_info.imageFormat      = m_swapchainSurfaceFormat.format;
		swapchain_create_info.imageColorSpace  = m_swapchainSurfaceFormat.colorSpace;
		swapchain_create_info.imageExtent      = m_swapchainExtent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
		swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
		swapchain_create_info.preTransform     = surface_caps.currentTransform;
		swapchain_create_info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchain_create_info.presentMode      = m_device->getPhysicalDevice()->chooseSwapchainPresentMode(*m_windowSurface);
		swapchain_create_info.clipped          = true;

		if (*m_swapchain)
		{
			// I think this should work, but I am not totally sure
			swapchain_create_info.oldSwapchain = *m_swapchain;
		}

		m_swapchain       = {m_device->getVulkanLogicalDevice(), swapchain_create_info};
		m_swapchainImages = m_swapchain.getImages();
	}

	auto VKSwapchain::_recreateSwapchain() -> void
	{
		// Blocks execution until the window is a valid size (for minimisation)

		// I just don't want to get GLFW involved at this point and would like to possibly make ts window api agnostic
		if (m_handleMinimisationCallback)
			m_handleMinimisationCallback();

		// Wait for the GPU to finish processing anything before recreating, so nothing that depends on the swapchain becomes invalid
		m_device->getVulkanLogicalDevice().waitIdle();

		m_swapchainImageViews.clear();

		_create();
		_createImageViews();
		_createDepthResources();

		if (m_resizeCallback)
			m_resizeCallback(m_swapchainExtent.width, m_swapchainExtent.height);

		m_device->getVulkanLogicalDevice().waitIdle();
	}
}
