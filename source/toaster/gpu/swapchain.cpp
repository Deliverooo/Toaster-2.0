#include "swapchain.hpp"

#include "gpu_context.hpp"
#include "toast_assert.h"

namespace toaster::gpu
{
	Swapchain::Swapchain(GPUContext *p_ctx, vk::SurfaceKHR p_window_surface) : m_gpuContext(p_ctx), m_surface(p_window_surface)
	{
	}

	void Swapchain::create(uint32 p_width, uint32 p_height)
	{
		m_width  = p_width;
		m_height = p_height;

		destroy();

		vk::Device vk_device = m_gpuContext->getLogicalDevice();

		GPUContext::SwapchainSupportDetails swapchain_support_details = m_gpuContext->querySwapchainSupport(m_gpuContext->getPhysicalDevice(), m_surface);

		m_swapchainFormat = {static_cast<vk::Format>(nvrhi::vulkan::convertFormat(nvrhi::Format::BGRA8_UNORM)), vk::ColorSpaceKHR::eSrgbNonlinear};

		vk::PresentModeKHR present_mode = m_gpuContext->choosePresentMode(swapchain_support_details.presentModes);

		std::unordered_set<uint32> unique_queues = {
			static_cast<uint32>(m_gpuContext->getQueueFamilyIndices().graphics),
			static_cast<uint32>(m_gpuContext->getQueueFamilyIndices().present)
		};
		std::vector<uint32> queues = {unique_queues.begin(), unique_queues.end()};

		const bool enable_swapchain_sharing = queues.size() > 1;

		uint32 image_count = swapchain_support_details.capabilities.minImageCount + 1u;
		if (image_count > swapchain_support_details.capabilities.maxImageCount && swapchain_support_details.capabilities.maxImageCount > 0)
		{
			image_count = swapchain_support_details.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.surface          = m_surface;
		swapchain_create_info.imageFormat      = m_swapchainFormat.format;
		swapchain_create_info.imageColorSpace  = m_swapchainFormat.colorSpace;
		swapchain_create_info.presentMode      = present_mode;
		swapchain_create_info.imageExtent      = vk::Extent2D(m_width, m_height);
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		swapchain_create_info.minImageCount    = image_count;

		swapchain_create_info.imageSharingMode      = enable_swapchain_sharing ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
		swapchain_create_info.queueFamilyIndexCount = enable_swapchain_sharing ? static_cast<uint32>(queues.size()) : 0u;
		swapchain_create_info.pQueueFamilyIndices   = enable_swapchain_sharing ? queues.data() : nullptr;

		swapchain_create_info.preTransform   = swapchain_support_details.capabilities.currentTransform;
		swapchain_create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchain_create_info.clipped        = vk::True;
		swapchain_create_info.oldSwapchain   = nullptr;

		bool mutable_format_supported = m_gpuContext->isDeviceExtensionEnabled(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME);

		swapchain_create_info.flags = mutable_format_supported ? vk::SwapchainCreateFlagBitsKHR::eMutableFormat : static_cast<vk::SwapchainCreateFlagBitsKHR>(0);

		std::vector<vk::Format> image_formats = {m_swapchainFormat.format};
		switch (m_swapchainFormat.format)
		{
			case vk::Format::eR8G8B8A8Unorm:
			{
				image_formats.push_back(vk::Format::eR8G8B8A8Srgb);
				break;
			}
			case vk::Format::eR8G8B8A8Srgb:
			{
				image_formats.push_back(vk::Format::eR8G8B8A8Unorm);
				break;
			}
			case vk::Format::eB8G8R8A8Unorm:
			{
				image_formats.push_back(vk::Format::eB8G8R8A8Srgb);
				break;
			}
			case vk::Format::eB8G8R8A8Srgb:
			{
				image_formats.push_back(vk::Format::eB8G8R8A8Unorm);
				break;
			}
			default: { break; }
		}

		vk::ImageFormatListCreateInfo image_format_list_create_info{};
		image_format_list_create_info.pViewFormats    = image_formats.data();
		image_format_list_create_info.viewFormatCount = static_cast<uint32>(image_formats.size());

		if (mutable_format_supported)
			swapchain_create_info.pNext = &image_format_list_create_info;

		m_swapchain = vk_device.createSwapchainKHR(swapchain_create_info);

		_createSwapchainImages();

		m_swapchainIndex = 0u;

		for (uint32 i = 0u; i < GPUContext::c_maxFramesInFlight; i++)
		{
			m_acquireSemaphores[i] = vk_device.createSemaphore(vk::SemaphoreCreateInfo{});
			// Create fences in signaled state so we don't wait on first use
			m_acquireFences[i] = vk_device.createFence(vk::FenceCreateInfo{vk::FenceCreateFlagBits::eSignaled});
		}

		m_presentSemaphores.resize(m_swapchainImages.size());
		for (uint64 i = 0u; i < m_swapchainImages.size(); i++)
		{
			m_presentSemaphores[i] = vk_device.createSemaphore(vk::SemaphoreCreateInfo{});
		}

		_createSwapchainFramebuffers();
	}

	void Swapchain::destroy()
	{
		vk::Device vk_device = m_gpuContext->getLogicalDevice();
		vk_device.waitIdle();

		if (m_swapchain)
		{
			vk_device.destroySwapchainKHR(m_swapchain);
			m_swapchain = nullptr;
		}

		m_swapchainImages.clear();

		for (auto &semaphore: m_presentSemaphores)
		{
			if (semaphore)
			{
				vk_device.destroySemaphore(semaphore);
				semaphore = nullptr;
			}
		}

		for (auto &semaphore: m_acquireSemaphores)
		{
			if (semaphore)
			{
				vk_device.destroySemaphore(semaphore);
				semaphore = nullptr;
			}
		}

		for (auto &fence: m_acquireFences)
		{
			if (fence)
			{
				vk_device.destroyFence(fence);
				fence = nullptr;
			}
		}
	}

	void Swapchain::onResize(uint32 p_width, uint32 p_height)
	{
		m_width  = p_width;
		m_height = p_height;

		m_swapchainFramebuffers.clear();

		resize();
	}

	void Swapchain::resize()
	{
		destroy();
		create(m_width, m_height);
	}

	vk::Result Swapchain::beginFrame()
	{
		auto       nv_device = dynamic_cast<nvrhi::vulkan::IDevice *>(m_gpuContext->getNVRHIDevice());
		vk::Device vk_device = m_gpuContext->getLogicalDevice();

		// Wait for the fence to ensure the semaphore from a previous frame is no longer in use
		auto &fence = m_acquireFences[m_acquireSemaphoreIndex];
		(void)vk_device.waitForFences(1, &fence, vk::True, std::numeric_limits<uint64>::max());
		vk_device.resetFences(1, &fence);

		const auto &semaphore = m_acquireSemaphores[m_acquireSemaphoreIndex];

		vk::Result result{};

		constexpr int32 c_maxAttempts = 3;
		for (int32 attempt = 0; attempt < c_maxAttempts; ++attempt)
		{
			result              = vk_device.acquireNextImageKHR(m_swapchain, std::numeric_limits<uint64>::max(), semaphore, nullptr, &m_swapchainIndex);
			m_acquiredSemaphore = semaphore;

			if (result == vk::Result::eErrorOutOfDateKHR && attempt < c_maxAttempts)
			{
				m_swapchainFramebuffers.clear();

				auto surface_caps = m_gpuContext->getPhysicalDevice().getSurfaceCapabilitiesKHR(m_surface);

				m_width  = surface_caps.currentExtent.width;
				m_height = surface_caps.currentExtent.height;

				resize();

				_createSwapchainFramebuffers();
			}
			else
			{
				break;
			}
		}

		// Store the index used for this frame so present() can signal the corresponding fence
		m_currentAcquireFenceIndex = m_acquireSemaphoreIndex;
		m_acquireSemaphoreIndex = (m_acquireSemaphoreIndex + 1) % m_acquireSemaphores.size();

		if (result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR) // Suboptimal is considered a success
		{
			// Schedule the wait. The actual wait operation will be submitted when the app executes any command list.
			nv_device->queueWaitForSemaphore(nvrhi::CommandQueue::Graphics, semaphore, 0);
			return vk::Result::eSuccess;
		}

		return result;
	}

	void Swapchain::present()
	{
		auto       nv_device = dynamic_cast<nvrhi::vulkan::IDevice *>(m_gpuContext->getNVRHIDevice());
		vk::Device vk_device = m_gpuContext->getLogicalDevice();

		nv_device->queueSignalSemaphore(nvrhi::CommandQueue::Graphics, m_presentSemaphores[m_swapchainIndex], 0);

		// NVRHI buffers the semaphores and signals them when something is submitted to a queue.
		// Call 'executeCommandLists' with no command lists to actually signal the semaphore.
		nv_device->executeCommandLists(nullptr, 0);

		// Submit an empty command buffer with the fence to track when the acquire semaphore is consumed
		vk::SubmitInfo submitInfo{};
		m_gpuContext->getGraphicsQueue().submit(1, &submitInfo, m_acquireFences[m_currentAcquireFenceIndex]);

		vk::PresentInfoKHR info{};
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores    = &m_presentSemaphores[m_swapchainIndex];
		info.swapchainCount     = 1;
		info.pSwapchains        = &m_swapchain;
		info.pImageIndices      = &m_swapchainIndex;

		vk::Result res = m_gpuContext->getPresentQueue().presentKHR(&info);
		TST_ASSERT(res == vk::Result::eSuccess || res == vk::Result::eErrorOutOfDateKHR);

		while (m_framesInFlight.size() >= GPUContext::c_maxFramesInFlight)
		{
			auto query = m_framesInFlight.front();
			m_framesInFlight.pop();

			nv_device->waitEventQuery(query);

			m_queryPool.push_back(query);
		}

		nvrhi::EventQueryHandle query;
		if (!m_queryPool.empty())
		{
			query = m_queryPool.back();
			m_queryPool.pop_back();
		}
		else
		{
			query = nv_device->createEventQuery();
		}

		nv_device->resetEventQuery(query);
		nv_device->setEventQuery(query, nvrhi::CommandQueue::Graphics);
		m_framesInFlight.push(query);
	}

	uint32 Swapchain::getCurrentFrameIndex() const
	{
		return m_swapchainIndex;
	}

	nvrhi::ITexture *Swapchain::getImage(uint32 p_frame_index)
	{
		if (p_frame_index < m_swapchainImages.size())
			return m_swapchainImages[p_frame_index].handle;
		return nullptr;
	}

	nvrhi::ITexture *Swapchain::getCurrentImage()
	{
		return m_swapchainImages[m_swapchainIndex].handle;
	}

	nvrhi::IFramebuffer *Swapchain::getFramebuffer(uint32 p_frame_index)
	{
		if (p_frame_index < m_swapchainFramebuffers.size())
			return m_swapchainFramebuffers[p_frame_index];
		return nullptr;
	}

	nvrhi::IFramebuffer *Swapchain::getCurrentFramebuffer()
	{
		return m_swapchainFramebuffers[m_swapchainIndex];
	}

	void Swapchain::_createSwapchainImages()
	{
		nvrhi::IDevice *nv_device = m_gpuContext->getNVRHIDevice();
		vk::Device      vk_device = m_gpuContext->getLogicalDevice();

		auto images = vk_device.getSwapchainImagesKHR(m_swapchain);
		for (auto image: images)
		{
			SwapchainImage sci;
			sci.image = image;

			nvrhi::TextureDesc textureDesc;
			textureDesc.width            = m_width;
			textureDesc.height           = m_height;
			textureDesc.format           = nvrhi::Format::BGRA8_UNORM;
			textureDesc.debugName        = "Swapchain image";
			textureDesc.initialState     = nvrhi::ResourceStates::Present;
			textureDesc.keepInitialState = true;
			textureDesc.isRenderTarget   = true;

			sci.handle = nv_device->createHandleForNativeTexture(nvrhi::ObjectTypes::VK_Image, nvrhi::Object(sci.image), textureDesc);
			m_swapchainImages.push_back(sci);
		}
	}

	void Swapchain::_createSwapchainFramebuffers()
	{
		nvrhi::IDevice *nv_device = m_gpuContext->getNVRHIDevice();

		const uint32 image_count = m_swapchainImages.size();
		m_swapchainFramebuffers.resize(image_count);
		for (size_t i = 0; i < image_count; i++)
		{
			m_swapchainFramebuffers[i] = nv_device->createFramebuffer(nvrhi::FramebufferDesc().addColorAttachment(getImage(i)));
		}
	}
}
