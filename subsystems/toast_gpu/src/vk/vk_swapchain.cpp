#include "toast_gpu/vk/vk_swapchain.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::gpu
{
	VKSwapchain::VKSwapchain(VKGPUContext &p_gpu_ctx, vk::SurfaceKHR *p_surface) : m_gpuCtx(&p_gpu_ctx), m_windowSurface(p_surface)
	{
		TST_ASSERT_MSG(p_dev, "Device cannot be null");

		for (uint32 i{0u}; i < m_gpuCtx->getSpecInfo().maxFramesInFlight; ++i)
		{
			m_commandBuffers.emplace_back(*m_gpuCtx, vk::QueueFlagBits::eGraphics, true);
		}
		_create();
		_createImageViews();
		_createSyncObjects();
		_createDepthResources();
	}

	VKSwapchain::~VKSwapchain()
	{
		for (auto &view: m_swapchainImageViews)
			m_gpuCtx->getLogicalDevice()->destroyObject(view);
		m_swapchainImageViews.clear();
	}

	auto VKSwapchain::beginFrame() -> void
	{
		auto &current_command_buffer{m_commandBuffers[m_frameIndex]};

		// Wait for the previous frame to be finished before rendering this one
		current_command_buffer.waitForFence();

		if (m_beginFrameCallback)
			m_beginFrameCallback(m_userData, m_frameIndex);

		// Reset the fence so we can signal it later
		current_command_buffer.resetFence();

		auto [res, image_index] = m_swapchain.acquireNextImage(UINT64_MAX, *m_imageAvailableSemaphores[m_frameIndex], nullptr);
		m_imageIndex            = image_index;

		if (res == vk::Result::eErrorOutOfDateKHR)
		{
			_recreateSwapchain();
			return;
		}
		if (res != vk::Result::eSuccess) TST_ASSERT_MSG(false, "Failed to acquire swapchain image!");

		current_command_buffer.resetCommandBuffer();
		current_command_buffer.begin();

		m_gpuCtx->transitionImageLayout(m_swapchainImages[m_imageIndex], c_swapchainEndFrameLayoutInfo, c_swapchainBeginFrameLayoutInfo, 1, 1,
										vk::ImageAspectFlagBits::eColor, current_command_buffer);

		m_gpuCtx->transitionImageLayout(m_depthImage, c_swapchainEndFrameDepthLayoutInfo, util::getImageLayoutInfo(vk::ImageLayout::eDepthAttachmentOptimal), 1, 1,
										vk::ImageAspectFlagBits::eDepth, current_command_buffer);
	}

	auto VKSwapchain::endFrame() -> void
	{
		auto &current_command_buffer{m_commandBuffers[m_frameIndex]};

		m_gpuCtx->transitionImageLayout(m_swapchainImages[m_imageIndex], c_swapchainBeginFrameLayoutInfo, c_swapchainPresentSrcLayoutInfo, 1, 1,
										vk::ImageAspectFlagBits::eColor, current_command_buffer);

		current_command_buffer.end();

		// Waits for the image to be acquired before executing
		// When we submit the work to the GPU we signal a fence then wait on it before beginning the next frame
		current_command_buffer.submit(vk::PipelineStageFlagBits2::eColorAttachmentOutput, {*m_imageAvailableSemaphores[m_frameIndex]},
									  {*m_renderFinishedSemaphores[m_imageIndex]});

		const vk::Result res{m_gpuCtx->getLogicalDevice()->presentKHR(&*m_swapchain, &m_imageIndex, {*m_renderFinishedSemaphores[m_imageIndex]})};
		if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || m_framebufferResized)
		{
			m_framebufferResized = false;
			_recreateSwapchain();
		}
		else if (res != vk::Result::eSuccess)
			TST_PERMA_ASSERT_MSG(false, "Failed to present swapchain image!");

		m_frameIndex = (m_frameIndex + 1) % m_gpuCtx->getSpecInfo().maxFramesInFlight;
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

	auto VKSwapchain::getImageView(uint32 p_index) -> vk::ImageView &
	{
		TST_ASSERT_MSG(p_index < m_swapchainImageViews.size(), "Out of bounds");
		return m_swapchainImageViews[p_index];
	}

	auto VKSwapchain::getCurrentImage() -> vk::Image &
	{
		return m_swapchainImages[m_imageIndex];
	}

	auto VKSwapchain::getCurrentImageView() -> vk::ImageView &
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

	auto VKSwapchain::getCommandBuffer(uint32 p_frame_index) -> VKCommandBuffer &
	{
		return m_commandBuffers.at(p_frame_index);
	}

	auto VKSwapchain::getCurrentCommandBuffer() -> VKCommandBuffer &
	{
		return m_commandBuffers.at(m_frameIndex);
	}

	auto VKSwapchain::getExtent() const -> vk::Extent2D
	{
		return m_swapchainExtent;
	}

	auto VKSwapchain::getAspectRatio() const -> float32
	{
		return m_swapchainExtent.width / m_swapchainExtent.height;
	}

	auto VKSwapchain::getSurfaceFormat() const -> vk::SurfaceFormatKHR
	{
		return m_swapchainSurfaceFormat;
	}

	auto VKSwapchain::getDepthFormat() const -> vk::Format
	{
		return m_gpuCtx->getPhysicalDevice()->getDepthFormat();
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

	auto VKSwapchain::setUserDataPointer(void *p_user_data) -> void
	{
		m_userData = p_user_data;
	}

	auto VKSwapchain::setResizeUserDataPointer(void *p_user_data) -> void
	{
		m_resizeData = p_user_data;
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
			m_swapchainImageViews.emplace_back(m_gpuCtx->getLogicalDevice()->createImageView(img, m_swapchainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1u,
																							 1u));
	}

	auto VKSwapchain::_createDepthResources() -> void
	{
		m_gpuCtx->getLogicalDevice()->createImage({m_swapchainExtent.width, m_swapchainExtent.height, 1u}, 1u, 1u, vk::SampleCountFlagBits::e1, getDepthFormat(),
												  vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
												  m_depthImage, m_depthImageMemory);
		m_depthImageView = m_gpuCtx->getLogicalDevice()->createImageView(m_depthImage, getDepthFormat(), vk::ImageAspectFlagBits::eDepth, 1u, 1u);
	}

	auto VKSwapchain::_createSyncObjects() -> void
	{
		for (uint32 i{0u}; i < m_gpuCtx->getSpecInfo().maxFramesInFlight; ++i)
		{
			// I don't know why vk::SemaphoreCreateInfo exists, there are no parameters that you can set for it
			m_imageAvailableSemaphores.emplace_back(m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice(), vk::SemaphoreCreateInfo{});
		}
		for (uint32 i{0u}; i < m_swapchainImages.size(); ++i)
		{
			m_renderFinishedSemaphores.emplace_back(m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice(), vk::SemaphoreCreateInfo{});
		}
	}

	auto VKSwapchain::_create() -> void
	{
		const auto physical_device{m_gpuCtx->getPhysicalDevice()};

		m_swapchainSurfaceFormat = physical_device->chooseSwapchainSurfaceFormat(*m_windowSurface);

		uint32 width{0u};
		uint32 height{0u};

		// The fallback extent will be equal to the back buffer's size
		if (m_getWindowBackBufferSizeCallback)
		{
			const auto [w, h]{m_getWindowBackBufferSizeCallback()};
			width  = w;
			height = h;
		}
		m_swapchainExtent = physical_device->chooseSwapchainExtent(*m_windowSurface, width, height);
		m_minImageCount   = physical_device->chooseSwapchainMinImageCount(*m_windowSurface);

		vk::SwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.surface          = *m_windowSurface;
		swapchain_create_info.minImageCount    = m_minImageCount;
		swapchain_create_info.imageFormat      = m_swapchainSurfaceFormat.format;
		swapchain_create_info.imageColorSpace  = m_swapchainSurfaceFormat.colorSpace;
		swapchain_create_info.imageExtent      = m_swapchainExtent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
		swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
		swapchain_create_info.preTransform     = physical_device->getVulkanPhysicalDevice().getSurfaceCapabilitiesKHR(*m_windowSurface).currentTransform;
		swapchain_create_info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchain_create_info.presentMode      = physical_device->chooseSwapchainPresentMode(*m_windowSurface);
		swapchain_create_info.clipped          = true;

		if (*m_swapchain)
		{
			// I think this should work, but I am not totally sure
			swapchain_create_info.oldSwapchain = *m_swapchain;
		}

		m_swapchain       = {m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice(), swapchain_create_info};
		m_swapchainImages = m_swapchain.getImages();
	}

	auto VKSwapchain::_recreateSwapchain() -> void
	{
		// Blocks execution until the window is a valid size (for minimisation)

		// I just don't want to get GLFW involved at this point and would like to possibly make ts window api agnostic
		if (m_handleMinimisationCallback)
			m_handleMinimisationCallback();

		// Wait for the GPU to finish processing anything before recreating, so nothing that depends on the swapchain becomes invalid
		m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice().waitIdle();

		for (auto &view: m_swapchainImageViews)
			m_gpuCtx->getLogicalDevice()->destroyObject(view);
		m_swapchainImageViews.clear();

		_create();
		_createImageViews();
		_createDepthResources();

		if (m_resizeCallback)
			m_resizeCallback(m_resizeData, {m_swapchainExtent.width, m_swapchainExtent.height});

		m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice().waitIdle();
	}
}
