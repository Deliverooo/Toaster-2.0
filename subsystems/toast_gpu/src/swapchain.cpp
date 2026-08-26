#include "toast_gpu/swapchain.hpp"

#include "toast_gpu/command_list.hpp"

namespace toaster::gpu
{
	Swapchain::Swapchain(Device &p_gpu_ctx, vk::SurfaceKHR p_surface, uint32 p_max_frames_in_flight, uint32 p_initial_width,
						 uint32  p_initial_height) : m_gpuCtx(&p_gpu_ctx), m_maxFramesInFlight(p_max_frames_in_flight), m_windowSurface(p_surface)
	{
		m_swapchainSurfaceFormat = m_gpuCtx->getPhysicalDevice().chooseSwapchainSurfaceFormat(m_windowSurface);
		m_depthFormat            = vk::Format::eD32Sfloat;
		m_minImageCount          = m_gpuCtx->getPhysicalDevice().chooseSwapchainMinImageCount(m_windowSurface);
		_create(p_initial_width, p_initial_height);

		for (uint32 i{0u}; i < m_images.size(); ++i)
			m_renderFinishedSemaphores.emplace_back(m_gpuCtx->getDevice().getDevice().createSemaphore(vk::SemaphoreCreateInfo{}));
	}

	Swapchain::~Swapchain()
	{
		auto &device{m_gpuCtx->getDevice()};

		device.getDevice().destroyImageView(m_depthImageView);
		m_gpuCtx->getAllocator().destroyImage(m_depthImage, m_depthImageAllocation);

		for (auto &image_view: m_imageViews)
			device.getDevice().destroyImageView(image_view);

		for (auto &semaphore: m_renderFinishedSemaphores)
			device.getDevice().destroySemaphore(semaphore);

		device.getDevice().destroySwapchainKHR(m_swapchain);
	}

	auto Swapchain::acquireImage(vk::Semaphore p_signal_semaphore) -> void
	{
		auto [res, image_index] = m_gpuCtx->getDevice().getDevice().acquireNextImageKHR(m_swapchain,UINT64_MAX, p_signal_semaphore, nullptr);
		m_imageIndex            = image_index;

		if (res == vk::Result::eErrorOutOfDateKHR)
			TST_PERMA_ASSERT(false);
		if (res != vk::Result::eSuccess)
			TST_PERMA_ASSERT_MSG(false, "Failed to acquire swapchain image!");
	}

	auto Swapchain::beginFrame(CommandList &p_cmd) -> void
	{
		vk::ImageMemoryBarrier2 undefined_to_colour_attachment_optimal{};
		undefined_to_colour_attachment_optimal.image               = m_images[m_imageIndex];
		undefined_to_colour_attachment_optimal.srcAccessMask       = vk::AccessFlagBits2::eNone;
		undefined_to_colour_attachment_optimal.dstAccessMask       = vk::AccessFlagBits2::eColorAttachmentWrite;
		undefined_to_colour_attachment_optimal.srcStageMask        = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		undefined_to_colour_attachment_optimal.dstStageMask        = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		undefined_to_colour_attachment_optimal.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		undefined_to_colour_attachment_optimal.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		undefined_to_colour_attachment_optimal.oldLayout           = vk::ImageLayout::eUndefined;
		undefined_to_colour_attachment_optimal.newLayout           = vk::ImageLayout::eColorAttachmentOptimal;
		undefined_to_colour_attachment_optimal.subresourceRange    = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u};

		vk::ImageMemoryBarrier2 undefined_to_depth_attachment_optimal{};
		undefined_to_depth_attachment_optimal.image               = m_depthImage;
		undefined_to_depth_attachment_optimal.srcAccessMask       = vk::AccessFlagBits2::eNone;
		undefined_to_depth_attachment_optimal.dstAccessMask       = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
		undefined_to_depth_attachment_optimal.srcStageMask        = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
		undefined_to_depth_attachment_optimal.dstStageMask        = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
		undefined_to_depth_attachment_optimal.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		undefined_to_depth_attachment_optimal.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		undefined_to_depth_attachment_optimal.oldLayout           = vk::ImageLayout::eUndefined;
		undefined_to_depth_attachment_optimal.newLayout           = vk::ImageLayout::eDepthAttachmentOptimal;
		undefined_to_depth_attachment_optimal.subresourceRange    = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth, 0u, 1u, 0u, 1u};

		vk::DependencyInfo      dependency_info{};
		vk::ImageMemoryBarrier2 barriers[]{undefined_to_colour_attachment_optimal, undefined_to_depth_attachment_optimal};
		dependency_info.setImageMemoryBarriers(barriers);
		p_cmd.getCommandBuffer().pipelineBarrier2(dependency_info);
	}

	auto Swapchain::endFrame(CommandList &p_cmd) -> void
	{
		vk::ImageMemoryBarrier2 colour_attachment_optimal_to_present_src{};
		colour_attachment_optimal_to_present_src.image               = m_images[m_imageIndex];
		colour_attachment_optimal_to_present_src.srcAccessMask       = vk::AccessFlagBits2::eColorAttachmentWrite;
		colour_attachment_optimal_to_present_src.dstAccessMask       = vk::AccessFlagBits2::eNone;
		colour_attachment_optimal_to_present_src.srcStageMask        = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		colour_attachment_optimal_to_present_src.dstStageMask        = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		colour_attachment_optimal_to_present_src.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		colour_attachment_optimal_to_present_src.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		colour_attachment_optimal_to_present_src.oldLayout           = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_optimal_to_present_src.newLayout           = vk::ImageLayout::ePresentSrcKHR;
		colour_attachment_optimal_to_present_src.subresourceRange    = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u};

		vk::DependencyInfo dependency_info{};
		dependency_info.setImageMemoryBarriers(colour_attachment_optimal_to_present_src);
		p_cmd.getCommandBuffer().pipelineBarrier2(dependency_info);
	}

	auto Swapchain::getSignalSemaphoreInfo() const -> vk::SemaphoreSubmitInfo
	{
		vk::SemaphoreSubmitInfo signal_semaphore_info{};
		signal_semaphore_info.semaphore = m_renderFinishedSemaphores[m_imageIndex];
		return signal_semaphore_info;
	}

	auto Swapchain::present() -> void
	{
		vk::PresentInfoKHR present_info{};
		present_info.setSwapchains(m_swapchain);
		present_info.setImageIndices(m_imageIndex);
		present_info.setWaitSemaphores(m_renderFinishedSemaphores[m_imageIndex]);
		vk::Result res{m_gpuCtx->getDevice().getGraphicsQueue().presentKHR(present_info)};

		if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR)
			TST_PERMA_ASSERT(false);
		else if (res != vk::Result::eSuccess)
			TST_PERMA_ASSERT_MSG(false, "Failed to present swapchain image!");
	}

	auto Swapchain::onResize(uint32 p_width, uint32 p_height) -> void
	{
		m_gpuCtx->getDevice().getDevice().waitIdle();
		_create(p_width, p_height);
	}

	auto Swapchain::getColourAttachmentInfo(const vk::ClearColorValue &p_clear_colour) const -> vk::RenderingAttachmentInfo
	{
		vk::RenderingAttachmentInfo info{};
		info.imageView   = m_imageViews[m_imageIndex];
		info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		info.loadOp      = vk::AttachmentLoadOp::eClear;
		info.storeOp     = vk::AttachmentStoreOp::eStore;
		info.clearValue  = p_clear_colour;
		return info;
	}

	auto Swapchain::getDepthAttachmentInfo(vk::ClearDepthStencilValue p_clear_value) const -> vk::RenderingAttachmentInfo
	{
		vk::RenderingAttachmentInfo info{};
		info.imageView   = m_depthImageView;
		info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		info.loadOp      = vk::AttachmentLoadOp::eClear;
		info.storeOp     = vk::AttachmentStoreOp::eStore;
		info.clearValue  = p_clear_value;
		return info;
	}

	auto Swapchain::_create(uint32 p_width, uint32 p_height) -> void
	{
		for (auto &image_view: m_imageViews)
			m_gpuCtx->getDevice().getDevice().destroyImageView(image_view);
		m_imageViews.clear();

		if (m_depthImage)
		{
			m_gpuCtx->getDevice().getDevice().destroyImageView(m_depthImageView);
			m_gpuCtx->getAllocator().destroyImage(m_depthImage, m_depthImageAllocation);
		}

		m_swapchainExtent = m_gpuCtx->getPhysicalDevice().chooseSwapchainExtent(m_windowSurface, p_width, p_height);

		vk::SwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.surface          = m_windowSurface;
		swapchain_create_info.minImageCount    = m_minImageCount;
		swapchain_create_info.imageFormat      = m_swapchainSurfaceFormat.format;
		swapchain_create_info.imageColorSpace  = m_swapchainSurfaceFormat.colorSpace;
		swapchain_create_info.imageExtent      = m_swapchainExtent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
		swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
		swapchain_create_info.preTransform     = m_gpuCtx->getPhysicalDevice().getPhysicalDevice().getSurfaceCapabilitiesKHR(m_windowSurface).currentTransform;
		swapchain_create_info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchain_create_info.presentMode      = m_gpuCtx->getPhysicalDevice().chooseSwapchainPresentMode(m_windowSurface);
		swapchain_create_info.clipped          = true;

		vk::SwapchainKHR old_swapchain{nullptr};
		if (m_swapchain)
			old_swapchain = m_swapchain;
		swapchain_create_info.oldSwapchain = old_swapchain;

		m_swapchain = m_gpuCtx->getDevice().getDevice().createSwapchainKHR(swapchain_create_info);

		if (old_swapchain)
			m_gpuCtx->getDevice().getDevice().destroySwapchainKHR(old_swapchain);

		m_images = m_gpuCtx->getDevice().getDevice().getSwapchainImagesKHR(m_swapchain);

		for (auto &img: m_images)
		{
			auto &                  view{m_imageViews.emplace_back()};
			vk::ImageViewCreateInfo image_view_create_info{};
			image_view_create_info.image      = img;
			image_view_create_info.viewType   = vk::ImageViewType::e2D;
			image_view_create_info.components = {
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity
			};
			image_view_create_info.subresourceRange = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1u, 0, 1u};
			image_view_create_info.format           = m_swapchainSurfaceFormat.format;
			view                                    = m_gpuCtx->getDevice().getDevice().createImageView(image_view_create_info);
		}

		vk::ImageCreateInfo depth_image_create_info{};
		depth_image_create_info.usage         = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		depth_image_create_info.format        = m_depthFormat;
		depth_image_create_info.sharingMode   = vk::SharingMode::eExclusive;
		depth_image_create_info.arrayLayers   = 1u;
		depth_image_create_info.mipLevels     = 1u;
		depth_image_create_info.extent        = vk::Extent3D{m_swapchainExtent, 1u};
		depth_image_create_info.imageType     = vk::ImageType::e2D;
		depth_image_create_info.samples       = vk::SampleCountFlagBits::e1;
		depth_image_create_info.initialLayout = vk::ImageLayout::eUndefined;
		depth_image_create_info.tiling        = vk::ImageTiling::eOptimal;

		m_gpuCtx->getAllocator().createImage(depth_image_create_info, 0u, m_depthImage, m_depthImageAllocation);

		vk::ImageViewCreateInfo depth_image_view_create_info{};
		depth_image_view_create_info.image      = m_depthImage;
		depth_image_view_create_info.components = vk::ComponentMapping{
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		};
		depth_image_view_create_info.format           = m_depthFormat;
		depth_image_view_create_info.subresourceRange = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth, 0u, 1u, 0u, 1u};
		depth_image_view_create_info.viewType         = vk::ImageViewType::e2D;
		m_depthImageView                              = m_gpuCtx->getDevice().getDevice().createImageView(depth_image_view_create_info);
	}
}
