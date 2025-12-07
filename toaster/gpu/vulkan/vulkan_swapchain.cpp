#include "vulkan_swapchain.hpp"

#include "vulkan_device_manager.hpp"
#include "gpu/command_buffer.hpp"
#include "nvrhi/vulkan.h"
#include "platform/application.hpp"

namespace tst
{
	VulkanSwapChain::VulkanSwapChain(vk::SurfaceKHR surface)
		: m_surface(surface)
	{
	}

	bool VulkanSwapChain::create(uint32 width, uint32 height)
	{
		m_width  = width;
		m_height = height;

		destroy();

		const auto &deviceParams = Application::getDeviceManager()->GetDeviceParams();
		auto        device       = Application::getGraphicsDevice();

		VulkanDeviceManager *vulkanDeviceManager = (VulkanDeviceManager *) Application::getDeviceManager();

		m_swapchainFormat = {vk::Format(nvrhi::vulkan::convertFormat(deviceParams.swapChainFormat)), vk::ColorSpaceKHR::eSrgbNonlinear};

		vk::Extent2D extent = vk::Extent2D(m_width, m_height);

		std::unordered_set<uint32_t> uniqueQueues = {
			uint32_t(vulkanDeviceManager->m_QueueFamilyIndices.Graphics), uint32_t(vulkanDeviceManager->m_QueueFamilyIndices.Present)
		};

		std::vector<uint32> queues;
		queues.reserve(uniqueQueues.size());
		std::ranges::transform(uniqueQueues, std::back_inserter(queues), [](uint32 q)
		{
			return q;
		});

		const bool enableSwapChainSharing = queues.size() > 1;

		vk::SwapchainCreateInfoKHR desc{};
		desc.surface          = m_surface;
		desc.minImageCount    = deviceParams.swapChainBufferCount;
		desc.imageFormat      = m_swapchainFormat.format;
		desc.imageColorSpace  = m_swapchainFormat.colorSpace;
		desc.imageExtent      = extent;
		desc.imageArrayLayers = 1;
		desc.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		desc.imageSharingMode = enableSwapChainSharing ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
		desc.flags            = (vulkanDeviceManager->m_SwapChainMutableFormatSupported
									 ? vk::SwapchainCreateFlagBitsKHR::eMutableFormat
									 : static_cast<vk::SwapchainCreateFlagBitsKHR>(0));
		desc.queueFamilyIndexCount = enableSwapChainSharing ? static_cast<uint32>(queues.size()) : 0;
		desc.pQueueFamilyIndices   = enableSwapChainSharing ? queues.data() : nullptr;
		desc.preTransform          = vk::SurfaceTransformFlagBitsKHR::eIdentity;
		desc.compositeAlpha        = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		desc.presentMode           = deviceParams.vsyncEnabled ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate;
		desc.clipped               = true;
		desc.oldSwapchain          = nullptr;

		std::vector<vk::Format> imageFormats = {m_swapchainFormat.format};
		switch (m_swapchainFormat.format)
		{
			case vk::Format::eR8G8B8A8Unorm:
			{
				imageFormats.push_back(vk::Format::eR8G8B8A8Srgb);
				break;
			}
			case vk::Format::eR8G8B8A8Srgb:
			{
				imageFormats.push_back(vk::Format::eR8G8B8A8Unorm);
				break;
			}
			case vk::Format::eB8G8R8A8Unorm:
			{
				imageFormats.push_back(vk::Format::eB8G8R8A8Srgb);
				break;
			}
			case vk::Format::eB8G8R8A8Srgb:
			{
				imageFormats.push_back(vk::Format::eB8G8R8A8Unorm);
				break;
			}
			default:
			{
				break;
			}
		}

		auto imageFormatListCreateInfo = vk::ImageFormatListCreateInfo().setViewFormats(imageFormats);

		if (vulkanDeviceManager->m_SwapChainMutableFormatSupported)
			desc.pNext = &imageFormatListCreateInfo;

		const vk::Result res = vulkanDeviceManager->m_VulkanDevice.createSwapchainKHR(&desc, nullptr, &m_swapchain);
		if (res != vk::Result::eSuccess)
		{
			TST_ERROR("Failed to create a Vulkan swap chain: {}", vk::to_string(res));
			return false;
		}

		// retrieve swap chain images
		auto images = vulkanDeviceManager->m_VulkanDevice.getSwapchainImagesKHR(m_swapchain);
		for (auto image: images)
		{
			SwapchainImage sci;
			sci.image = image;

			nvrhi::TextureDesc textureDesc;
			textureDesc.width            = m_width;
			textureDesc.height           = m_height;
			textureDesc.format           = deviceParams.swapChainFormat;
			textureDesc.debugName        = "Swap chain image";
			textureDesc.initialState     = nvrhi::ResourceStates::Present;
			textureDesc.keepInitialState = true;
			textureDesc.isRenderTarget   = true;

			sci.handle = device->createHandleForNativeTexture(nvrhi::ObjectTypes::VK_Image, nvrhi::Object(sci.image), textureDesc);
			m_swapchainImages.push_back(sci);
		}

		m_swapchainIndex = 0;

		for (uint32_t i = 0; i < 3; ++i)
		{
			m_presentSemaphores[i] = vulkanDeviceManager->m_VulkanDevice.createSemaphore(vk::SemaphoreCreateInfo());
			m_acquireSemaphores[i] = vulkanDeviceManager->m_VulkanDevice.createSemaphore(vk::SemaphoreCreateInfo());
		}

		backBufferResized();
	}

	void VulkanSwapChain::destroy()
	{
		auto *vulkanDeviceManager = dynamic_cast<VulkanDeviceManager *>(Application::getDeviceManager());
		if (vulkanDeviceManager->m_VulkanDevice)
		{
			vulkanDeviceManager->m_VulkanDevice.waitIdle();
		}

		if (m_swapchain)
		{
			vulkanDeviceManager->m_VulkanDevice.destroySwapchainKHR(m_swapchain);
			m_swapchain = nullptr;
		}

		m_swapchainImages.clear();

		for (auto &semaphore: m_presentSemaphores)
		{
			if (semaphore)
			{
				vulkanDeviceManager->m_VulkanDevice.destroySemaphore(semaphore);
				semaphore = vk::Semaphore();
			}
		}

		for (auto &semaphore: m_acquireSemaphores)
		{
			if (semaphore)
			{
				vulkanDeviceManager->m_VulkanDevice.destroySemaphore(semaphore);
				semaphore = vk::Semaphore();
			}
		}
	}

	void VulkanSwapChain::onResize(uint32 width, uint32 height)
	{
		m_width  = width;
		m_height = height;
		backBufferResizing();
		resize();
	}

	void VulkanSwapChain::resize()
	{
		destroy();
		create(m_width, m_height);
	}

	bool VulkanSwapChain::beginFrame()
	{
		auto  device              = dynamic_cast<nvrhi::vulkan::IDevice *>(Application::getGraphicsDevice().Get());
		auto *vulkanDeviceManager = dynamic_cast<VulkanDeviceManager *>(Application::getDeviceManager());

		const auto &semaphore = m_acquireSemaphores[m_acquireSemaphoreIndex];

		vk::Result res = {};

		int const maxAttempts = 3;
		for (int attempt = 0; attempt < maxAttempts; ++attempt)
		{
			res = vulkanDeviceManager->m_VulkanDevice.acquireNextImageKHR(m_swapchain, std::numeric_limits<uint64_t>::max(), // timeout
																		  semaphore, vk::Fence(), &m_swapchainIndex);

			m_acquiredSemaphore = semaphore;

			if (res == vk::Result::eErrorOutOfDateKHR && attempt < maxAttempts)
			{
				backBufferResizing();
				auto surfaceCaps = vulkanDeviceManager->m_VulkanPhysicalDevice.getSurfaceCapabilitiesKHR(m_surface);

				m_width  = surfaceCaps.currentExtent.width;
				m_height = surfaceCaps.currentExtent.height;

				resize();
				backBufferResized();
			}
			else
			{
				break;
			}
		}

		m_acquireSemaphoreIndex = (m_acquireSemaphoreIndex + 1) % m_acquireSemaphores.size();

		return res == vk::Result::eSuccess;
	}

	void VulkanSwapChain::present()
	{
		VulkanDeviceManager *vulkanDeviceManager = static_cast<VulkanDeviceManager *>(Application::getDeviceManager());
		auto                 device              = (nvrhi::vulkan::IDevice *) Application::getGraphicsDevice().Get();
		const auto &         semaphore           = m_presentSemaphores[m_presentSemaphoreIndex];

		CommandBuffer::lockGraphicsQueue();
		device->queueSignalSemaphore(nvrhi::CommandQueue::Graphics, semaphore, 0);
		device->executeCommandLists(nullptr, 0);

		vk::PresentInfoKHR info = vk::PresentInfoKHR().setWaitSemaphoreCount(1).setPWaitSemaphores(&semaphore).setSwapchainCount(1).setPSwapchains(&m_swapchain).
				setPImageIndices(&m_swapchainIndex);

		const vk::Result res = vulkanDeviceManager->m_PresentQueue.presentKHR(&info);
		TST_ASSERT(res == vk::Result::eSuccess || res == vk::Result::eErrorOutOfDateKHR);

		m_presentSemaphoreIndex = (m_presentSemaphoreIndex + 1) % m_presentSemaphores.size();

		CommandBuffer::unlockGraphicsQueue();

		#ifndef _WIN32
		if (deviceParams.vsyncEnabled)
		{
			m_presentQueue.waitIdle();
		}
		#endif

		{
			while (m_framesInFlight.size() >= vulkanDeviceManager->m_DeviceParams.maxFramesInFlight)
			{
				auto query = m_framesInFlight.front();
				m_framesInFlight.pop();

				device->waitEventQuery(query);

				m_queryPool.push_back(query);
			}
		}

		{
			nvrhi::EventQueryHandle query;
			if (!m_queryPool.empty())
			{
				query = m_queryPool.back();
				m_queryPool.pop_back();
			}
			else
			{
				query = device->createEventQuery();
			}

			device->resetEventQuery(query);
			device->setEventQuery(query, nvrhi::CommandQueue::Graphics);
			m_framesInFlight.push(query);
		}
	}

	nvrhi::IFramebuffer *VulkanSwapChain::getCurrentFramebuffer()
	{
		return getFramebuffer(getCurrentBackBufferIndex());
	}

	nvrhi::IFramebuffer *VulkanSwapChain::getFramebuffer(uint32_t index)
	{
		if (index < m_swapchainFramebuffers.size())
		{
			return m_swapchainFramebuffers[index];
		}

		return nullptr;
	}

	nvrhi::ITexture *VulkanSwapChain::getCurrentBackBuffer()
	{
		return m_swapchainImages[m_swapchainIndex].handle;
	}

	nvrhi::ITexture *VulkanSwapChain::getBackBuffer(uint32 index)
	{
		if (index < m_swapchainImages.size())
			return m_swapchainImages[index].handle;
		return nullptr;
	}

	uint32 VulkanSwapChain::getCurrentBackBufferIndex() const
	{
		return m_swapchainIndex;
	}

	uint32 VulkanSwapChain::getBackBufferCount()
	{
		return m_swapchainImages.size();
	}

	void VulkanSwapChain::backBufferResizing()
	{
		m_swapchainFramebuffers.clear();
	}

	void VulkanSwapChain::backBufferResized()
	{
		auto     device          = Application::getGraphicsDevice();
		uint32_t backBufferCount = getBackBufferCount();
		m_swapchainFramebuffers.resize(backBufferCount);
		for (uint32_t index = 0; index < backBufferCount; index++)
		{
			m_swapchainFramebuffers[index] = device->createFramebuffer(nvrhi::FramebufferDesc().addColorAttachment(getBackBuffer(index)));
		}
	}
}
