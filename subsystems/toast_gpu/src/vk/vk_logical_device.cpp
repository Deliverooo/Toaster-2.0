#include "toast_gpu/vk/vk_logical_device.hpp"

#include <ranges>

#include "toast_gpu/vk/vk_command_buffer.hpp"

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

namespace toaster::gpu
{
	VKLogicalDevice::VKLogicalDevice(VKPhysicalDevice &p_physical_device, const VKLogicalDeviceSpecInfo &p_spec_info) : m_physicalDevice(&p_physical_device),
																														m_specInfo(p_spec_info)
	{
		const auto queue_family_props = m_physicalDevice->getVulkanPhysicalDevice().getQueueFamilyProperties();

		const bool use_present{m_specInfo.requiredExtensions.contains(vk::KHRSwapchainExtensionName)};
		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			if (queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics && use_present
					? (vkGetPhysicalDeviceWin32PresentationSupportKHR(*m_physicalDevice->getVulkanPhysicalDevice(), i))
					: true)
			{
				m_queueFamilyIndices.graphics = i;
				break;
			}
		}
		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			if (queue_family_props[i].queueFlags & vk::QueueFlagBits::eCompute && queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				m_queueFamilyIndices.compute = i;
				break;
			}
		}
		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			if (queue_family_props[i].queueFlags & vk::QueueFlagBits::eTransfer && (
					static_cast<uint32>(queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics) == 0) && (
					static_cast<uint32>(queue_family_props[i].queueFlags & vk::QueueFlagBits::eCompute) == 0))
			{
				m_queueFamilyIndices.transfer = i;
				break;
			}
		}

		if (m_queueFamilyIndices.graphics == UINT32_MAX)
		{
			LOG_ERROR("Failed to find a queue family that supports present");
			TST_ASSERT(false);
		}

		if (m_queueFamilyIndices.transfer == UINT32_MAX)
		{
			LOG_ERROR("Failed to find a transfer queue family");
			TST_ASSERT(false);
		}

		if (m_queueFamilyIndices.compute == UINT32_MAX)
		{
			LOG_ERROR("Failed to find a compute queue family");
			TST_ASSERT(false);
		}

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};

		bool has_separate_compute_queue = m_queueFamilyIndices.graphics != m_queueFamilyIndices.compute;

		// Create the graphics and present queue
		// The graphics queue should be the same as the present one
		auto &graphics_queue_create_info            = queue_create_infos.emplace_back();
		graphics_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.graphics;
		graphics_queue_create_info.queueCount       = has_separate_compute_queue ? 1 : 2;
		std::vector<float32> queue_priorities;
		queue_priorities.emplace_back(1.0f);
		if (!has_separate_compute_queue)
			queue_priorities.emplace_back(1.0f);
		graphics_queue_create_info.pQueuePriorities = queue_priorities.data();

		constexpr float32 queue_priority = 1.0f;

		// Create the transfer queue
		auto &transfer_queue_create_info            = queue_create_infos.emplace_back();
		transfer_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.transfer;
		transfer_queue_create_info.queueCount       = 1;
		transfer_queue_create_info.pQueuePriorities = &queue_priority;

		if (has_separate_compute_queue)
		{
			// Create the compute queue
			auto &compute_queue_create_info            = queue_create_infos.emplace_back();
			compute_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.compute;
			compute_queue_create_info.queueCount       = 1;
			compute_queue_create_info.pQueuePriorities = &queue_priority;
		}

		std::vector<CString> extension_vec;
		for (const auto &ext: m_specInfo.requiredExtensions)
			extension_vec.emplace_back(ext.c_str());

		vk::DeviceCreateInfo device_create_info{};
		device_create_info.enabledExtensionCount   = static_cast<uint32>(extension_vec.size());
		device_create_info.ppEnabledExtensionNames = extension_vec.data();
		device_create_info.queueCreateInfoCount    = queue_create_infos.size();
		device_create_info.pQueueCreateInfos       = queue_create_infos.data();
		device_create_info.pNext                   = m_specInfo.pNext;

		m_logicalDevice = {m_physicalDevice->getVulkanPhysicalDevice(), device_create_info};

		// Create the queues
		m_graphicsQueue = {m_logicalDevice, m_queueFamilyIndices.graphics, 0};
		m_transferQueue = {m_logicalDevice, m_queueFamilyIndices.transfer, 0};
		if (m_queueFamilyIndices.graphics == m_queueFamilyIndices.compute)
			m_computeQueue = {m_logicalDevice, m_queueFamilyIndices.compute, 1};

		// if (m_physicalDevice->getInstance()->().printDebugInfo)
		// {
		// LOG_TRACE("Graphics queue family index {}", m_queueFamilyIndices.graphics);
		// LOG_TRACE("Transfer queue family index {}", m_queueFamilyIndices.transfer);
		// LOG_TRACE("Compute queue family index {}", m_queueFamilyIndices.compute);
		// }
		#pragma region create command pools
		// Graphics
		vk::CommandPoolCreateInfo graphics_command_pool_create_info{};
		graphics_command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.graphics;
		graphics_command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_graphicsCommandPool = {m_logicalDevice, graphics_command_pool_create_info};

		// Transfer
		vk::CommandPoolCreateInfo transfer_command_pool_create_info{};
		transfer_command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.transfer;
		transfer_command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_transferCommandPool = {m_logicalDevice, transfer_command_pool_create_info};

		// Compute
		vk::CommandPoolCreateInfo compute_command_pool_create_info{};
		compute_command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.compute;
		compute_command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_computeCommandPool = {m_logicalDevice, compute_command_pool_create_info};

		#pragma endregion
	}

	auto VKLogicalDevice::getPhysicalDevice() const -> NonOwningPtr<VKPhysicalDevice>
	{
		return m_physicalDevice;
	}

	auto VKLogicalDevice::getSpecInfo() const -> const VKLogicalDeviceSpecInfo &
	{
		return m_specInfo;
	}

	auto VKLogicalDevice::getVulkanLogicalDevice() -> vk::raii::Device &
	{
		return m_logicalDevice;
	}

	auto VKLogicalDevice::getQueueFamilyIndices() const -> const QueueFamilyIndices &
	{
		return m_queueFamilyIndices;
	}

	auto VKLogicalDevice::getQueueFamilyIndices(vk::QueueFlags p_queue_flags) const -> std::unordered_set<uint32>
	{
		std::unordered_set<uint32> queue_family_indices;

		if (p_queue_flags & vk::QueueFlagBits::eGraphics)
			queue_family_indices.emplace(m_queueFamilyIndices.graphics);
		if (p_queue_flags & vk::QueueFlagBits::eCompute)
			queue_family_indices.emplace(m_queueFamilyIndices.compute);
		if (p_queue_flags & vk::QueueFlagBits::eTransfer)
			queue_family_indices.emplace(m_queueFamilyIndices.transfer);

		return queue_family_indices;
	}

	auto VKLogicalDevice::getGraphicsQueue() -> vk::raii::Queue &
	{
		return m_graphicsQueue;
	}

	auto VKLogicalDevice::getTransferQueue() -> vk::raii::Queue &
	{
		return m_transferQueue;
	}

	auto VKLogicalDevice::getComputeQueue() -> vk::raii::Queue &
	{
		return m_computeQueue;
	}

	auto VKLogicalDevice::getQueue(vk::QueueFlagBits p_queue_type) -> vk::raii::Queue &
	{
		switch (p_queue_type)
		{
			case vk::QueueFlagBits::eGraphics: return m_graphicsQueue;
				break;
			case vk::QueueFlagBits::eCompute: return m_computeQueue;
				break;
			case vk::QueueFlagBits::eTransfer: return m_transferQueue;
				break;
			// case vk::QueueFlagBits::eSparseBinding:
			// case vk::QueueFlagBits::eProtected:
			// case vk::QueueFlagBits::eVideoDecodeKHR:
			// case vk::QueueFlagBits::eVideoEncodeKHR:
			// case vk::QueueFlagBits::eOpticalFlowNV:
			// case vk::QueueFlagBits::eDataGraphARM:
			default: break;
		}
		TST_ASSERT_MSG(false, "Unsupported queue");
		return m_graphicsQueue;
	}

	auto VKLogicalDevice::getGraphicsCommandPool() -> vk::raii::CommandPool &
	{
		return m_graphicsCommandPool;
	}

	auto VKLogicalDevice::getTransferCommandPool() -> vk::raii::CommandPool &
	{
		return m_transferCommandPool;
	}

	auto VKLogicalDevice::getComputeCommandPool() -> vk::raii::CommandPool &
	{
		return m_computeCommandPool;
	}

	auto VKLogicalDevice::getCommandPool(vk::QueueFlagBits p_queue_type) -> vk::raii::CommandPool &
	{
		switch (p_queue_type)
		{
			case vk::QueueFlagBits::eGraphics: return m_graphicsCommandPool;
				break;
			case vk::QueueFlagBits::eCompute: return m_computeCommandPool;
				break;
			case vk::QueueFlagBits::eTransfer: return m_transferCommandPool;
				break;
			// case vk::QueueFlagBits::eSparseBinding:
			// case vk::QueueFlagBits::eProtected:
			// case vk::QueueFlagBits::eVideoDecodeKHR:
			// case vk::QueueFlagBits::eVideoEncodeKHR:
			// case vk::QueueFlagBits::eOpticalFlowNV:
			// case vk::QueueFlagBits::eDataGraphARM:
			default: break;
		}
		TST_ASSERT_MSG(false, "Unsupported queue");
		return m_graphicsCommandPool;
	}

	auto VKLogicalDevice::presentKHR(const vk::SwapchainKHR *p_swapchain, const uint32 *p_image_indices, std::vector<vk::Semaphore> p_wait_semaphores) const -> vk::Result
	{
		vk::PresentInfoKHR present_info{};
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = p_wait_semaphores.data();
		present_info.swapchainCount     = 1;
		present_info.pSwapchains        = p_swapchain;
		present_info.pImageIndices      = p_image_indices;

		// For some reason, Vulkan-hpp classifies vk::Result::eErrorOutOfDateKHR as an error and automatically throws an exception
		// So this is what I came up with to bypass that :)
		return static_cast<vk::Result>(m_graphicsQueue.getDispatcher()->vkQueuePresentKHR(static_cast<VkQueue>(*m_graphicsQueue),
																						  reinterpret_cast<const VkPresentInfoKHR *>(&present_info)));
	}

	auto VKLogicalDevice::waitForFence(const vk::Fence &p_fence, uint64 p_timeout) const -> void
	{
		if (const vk::Result fence_result{m_logicalDevice.waitForFences({p_fence}, true, p_timeout)}; fence_result != vk::Result::eSuccess)
		{
			TST_ASSERT_MSG(false, "Failed to wait for Fence");
		}
	}

	auto VKLogicalDevice::waitForFences(const std::initializer_list<const vk::Fence> &p_fences, bool p_wait_all, uint64 p_timeout) const -> void
	{
		if (const vk::Result fence_result{m_logicalDevice.waitForFences({p_fences}, p_wait_all, p_timeout)}; fence_result != vk::Result::eSuccess)
		{
			TST_ASSERT_MSG(false, "Failed to wait for Fences");
		}
	}

	auto VKLogicalDevice::createShaderModule(const std::vector<uint8> &p_code) -> vk::raii::ShaderModule
	{
		vk::ShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.codeSize = p_code.size();
		shader_module_create_info.pCode    = reinterpret_cast<const uint32 *>(p_code.data());

		return {m_logicalDevice, shader_module_create_info};
	}

	auto VKLogicalDevice::createShaderModule(const std::vector<uint32> &p_code) -> vk::raii::ShaderModule
	{
		vk::ShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.codeSize = p_code.size() * sizeof(uint32);
		shader_module_create_info.pCode    = p_code.data();

		return {m_logicalDevice, shader_module_create_info};
	}

	auto VKLogicalDevice::mapMemory(vk::DeviceMemory p_memory, vk::DeviceSize p_offset, vk::DeviceSize p_size, vk::MemoryMapFlags p_flags) const -> void *
	{
		return (static_cast<vk::Device>(m_logicalDevice)).mapMemory(p_memory, p_offset, p_size, p_flags);
	}

	auto VKLogicalDevice::unmapMemory(vk::DeviceMemory p_memory) const -> void
	{
		(static_cast<vk::Device>(m_logicalDevice)).unmapMemory(p_memory);
	}

	auto VKLogicalDevice::createBuffer(vk::raii::Buffer &p_out_buffer, vk::raii::DeviceMemory &p_out_memory, vk::DeviceSize p_size, vk::BufferUsageFlags2 p_usage_flags,
									   vk::MemoryPropertyFlags p_memory_properties, vk::QueueFlags p_queue_access_flags) -> void
	{
		vk::BufferCreateInfo buffer_create_info{};
		buffer_create_info.size = p_size;

		const auto qfi{getQueueFamilyIndices(p_queue_access_flags)};
		buffer_create_info.queueFamilyIndexCount = qfi.size();
		auto qfi_vec{qfi | std::ranges::to<std::vector>()};
		buffer_create_info.pQueueFamilyIndices = qfi_vec.data();
		buffer_create_info.sharingMode         = (qfi.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive);

		vk::BufferUsageFlags2CreateInfo buffer_flags_create_info{};
		buffer_flags_create_info.usage = p_usage_flags;
		buffer_create_info.pNext       = &buffer_flags_create_info;

		p_out_buffer = {m_logicalDevice, buffer_create_info};

		vk::MemoryRequirements memory_requirements = p_out_buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.memoryTypeIndex = m_physicalDevice->findMemoryType(memory_requirements.memoryTypeBits, p_memory_properties);
		memory_allocate_info.allocationSize  = memory_requirements.size;

		vk::MemoryAllocateFlagsInfo memory_allocate_flags_info{};
		memory_allocate_flags_info.flags = (p_usage_flags & vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR)
											   ? vk::MemoryAllocateFlagBits::eDeviceAddressKHR
											   : static_cast<vk::MemoryAllocateFlagBits>(0u);
		memory_allocate_info.pNext = &memory_allocate_flags_info;

		p_out_memory = {m_logicalDevice, memory_allocate_info};

		p_out_buffer.bindMemory(p_out_memory, 0u);
	}

	auto VKLogicalDevice::createBuffer(vk::Buffer &            p_out_buffer, vk::DeviceMemory &p_out_memory, vk::DeviceSize p_size, vk::BufferUsageFlags2 p_usage_flags,
									   vk::MemoryPropertyFlags p_memory_properties, vk::QueueFlags p_queue_access_flags) const -> void
	{
		vk::BufferCreateInfo buffer_create_info{};
		buffer_create_info.size = p_size;

		const auto qfi{getQueueFamilyIndices(p_queue_access_flags)};
		buffer_create_info.queueFamilyIndexCount = qfi.size();
		auto qfi_vec{qfi | std::ranges::to<std::vector>()};
		buffer_create_info.pQueueFamilyIndices = qfi_vec.data();
		buffer_create_info.sharingMode         = (qfi.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive);

		vk::BufferUsageFlags2CreateInfo buffer_flags_create_info{};
		buffer_flags_create_info.usage = p_usage_flags;
		buffer_create_info.pNext       = &buffer_flags_create_info;

		p_out_buffer = (static_cast<vk::Device>(m_logicalDevice)).createBuffer(buffer_create_info);

		vk::MemoryRequirements memory_requirements = (static_cast<vk::Device>(m_logicalDevice)).getBufferMemoryRequirements(p_out_buffer);
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.memoryTypeIndex = m_physicalDevice->findMemoryType(memory_requirements.memoryTypeBits, p_memory_properties);
		memory_allocate_info.allocationSize  = memory_requirements.size;

		p_out_memory = (static_cast<vk::Device>(m_logicalDevice)).allocateMemory(memory_allocate_info);

		(static_cast<vk::Device>(m_logicalDevice)).bindBufferMemory(p_out_buffer, p_out_memory, 0u);
	}

	auto VKLogicalDevice::createImage(const ImageExtent &p_image_extent, uint32 p_layer_count, uint32 p_mip_levels, vk::SampleCountFlagBits p_sample_count,
									  vk::Format p_format, vk::ImageTiling p_image_tiling, vk::ImageUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties,
									  vk::raii::Image &p_out_image, vk::raii::DeviceMemory &p_out_memory) -> void
	{
		vk::ImageCreateInfo image_create_info{};
		image_create_info.extent        = p_image_extent;
		image_create_info.mipLevels     = p_mip_levels;
		image_create_info.arrayLayers   = p_layer_count;
		image_create_info.imageType     = vk::ImageType::e2D;
		image_create_info.samples       = p_sample_count;
		image_create_info.sharingMode   = vk::SharingMode::eConcurrent;
		image_create_info.tiling        = p_image_tiling;
		image_create_info.initialLayout = vk::ImageLayout::eUndefined;
		image_create_info.usage         = p_usage_flags;
		image_create_info.format        = p_format;
		image_create_info.flags         = (p_layer_count > 1) ? vk::ImageCreateFlagBits::eCubeCompatible : static_cast<vk::ImageCreateFlagBits>(0);

		image_create_info.queueFamilyIndexCount = 2;
		uint32 qfi[]                            = {m_queueFamilyIndices.graphics, m_queueFamilyIndices.transfer};
		image_create_info.pQueueFamilyIndices   = qfi;

		p_out_image                                = {m_logicalDevice, image_create_info};
		vk::MemoryRequirements memory_requirements = p_out_image.getMemoryRequirements();
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.allocationSize  = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = m_physicalDevice->findMemoryType(memory_requirements.memoryTypeBits, p_memory_properties);

		p_out_memory = {m_logicalDevice, memory_allocate_info};

		p_out_image.bindMemory(p_out_memory, 0u);
	}

	auto VKLogicalDevice::createImage(const ImageExtent &p_image_extent, uint32 p_layer_count, uint32 p_mip_levels, vk::SampleCountFlagBits p_sample_count,
									  vk::Format p_format, vk::ImageTiling p_image_tiling, vk::ImageUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties,
									  vk::Image &p_out_image, vk::DeviceMemory &p_out_memory) const -> void
	{
		vk::ImageCreateInfo image_create_info{};
		image_create_info.extent        = p_image_extent;
		image_create_info.mipLevels     = p_mip_levels;
		image_create_info.arrayLayers   = p_layer_count;
		image_create_info.imageType     = vk::ImageType::e2D;
		image_create_info.samples       = p_sample_count;
		image_create_info.sharingMode   = vk::SharingMode::eConcurrent;
		image_create_info.tiling        = p_image_tiling;
		image_create_info.initialLayout = vk::ImageLayout::eUndefined;
		image_create_info.usage         = p_usage_flags;
		image_create_info.format        = p_format;
		image_create_info.flags         = (p_layer_count > 1) ? vk::ImageCreateFlagBits::eCubeCompatible : static_cast<vk::ImageCreateFlagBits>(0);

		image_create_info.queueFamilyIndexCount = 2;
		uint32 qfi[]                            = {m_queueFamilyIndices.graphics, m_queueFamilyIndices.transfer};
		image_create_info.pQueueFamilyIndices   = qfi;

		p_out_image                                = (static_cast<vk::Device>(m_logicalDevice)).createImage(image_create_info);
		vk::MemoryRequirements memory_requirements = (static_cast<vk::Device>(m_logicalDevice)).getImageMemoryRequirements(p_out_image);
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.allocationSize  = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = m_physicalDevice->findMemoryType(memory_requirements.memoryTypeBits, p_memory_properties);

		p_out_memory = (static_cast<vk::Device>(m_logicalDevice)).allocateMemory(memory_allocate_info);

		(static_cast<vk::Device>(m_logicalDevice)).bindImageMemory(p_out_image, p_out_memory, 0u);
	}

	auto VKLogicalDevice::createImageView(vk::raii::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags, uint32 p_mip_levels,
										  uint32           p_layer_count) -> vk::raii::ImageView
	{
		vk::ImageViewCreateInfo image_view_create_info{};
		image_view_create_info.viewType   = (p_layer_count > 1) ? vk::ImageViewType::eCube : vk::ImageViewType::e2D;;
		image_view_create_info.image      = p_src_image;
		image_view_create_info.components = {
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		};
		image_view_create_info.subresourceRange = vk::ImageSubresourceRange{p_aspect_flags, 0, p_mip_levels, 0, p_layer_count};
		image_view_create_info.format           = p_format;

		return {m_logicalDevice, image_view_create_info};
	}

	auto VKLogicalDevice::createImageView(vk::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags, uint32 p_layer_count,
										  uint32     p_mip_levels) const -> vk::ImageView
	{
		vk::ImageViewCreateInfo image_view_create_info{};
		image_view_create_info.viewType   = (p_layer_count > 1) ? vk::ImageViewType::eCube : vk::ImageViewType::e2D;;
		image_view_create_info.image      = p_src_image;
		image_view_create_info.components = {
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		};
		image_view_create_info.subresourceRange = vk::ImageSubresourceRange{p_aspect_flags, 0, p_mip_levels, 0, p_layer_count};
		image_view_create_info.format           = p_format;

		return (static_cast<vk::Device>(m_logicalDevice)).createImageView(image_view_create_info);
	}

	auto VKLogicalDevice::createSamplerRaii(vk::Filter p_filter, vk::SamplerAddressMode p_address_mode) -> vk::raii::Sampler
	{
		const auto physical_device_props = m_physicalDevice->getVulkanPhysicalDevice().getProperties();

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.magFilter               = p_filter;
		sampler_create_info.minFilter               = p_filter;
		sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
		sampler_create_info.addressModeU            = p_address_mode;
		sampler_create_info.addressModeV            = p_address_mode;
		sampler_create_info.addressModeW            = p_address_mode;
		sampler_create_info.mipLodBias              = 0.0f;
		sampler_create_info.anisotropyEnable        = true;
		sampler_create_info.maxAnisotropy           = physical_device_props.limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable           = false;
		sampler_create_info.compareOp               = vk::CompareOp::eAlways;
		sampler_create_info.minLod                  = 0.0f;
		sampler_create_info.maxLod                  = vk::LodClampNone;
		sampler_create_info.borderColor             = vk::BorderColor::eFloatOpaqueWhite;
		sampler_create_info.unnormalizedCoordinates = false;

		return {m_logicalDevice, sampler_create_info};
	}

	auto VKLogicalDevice::createSampler(vk::Filter p_filter, vk::SamplerAddressMode p_address_mode) -> vk::Sampler
	{
		const auto physical_device_props = m_physicalDevice->getVulkanPhysicalDevice().getProperties();

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.magFilter               = p_filter;
		sampler_create_info.minFilter               = p_filter;
		sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
		sampler_create_info.addressModeU            = p_address_mode;
		sampler_create_info.addressModeV            = p_address_mode;
		sampler_create_info.addressModeW            = p_address_mode;
		sampler_create_info.mipLodBias              = 0.0f;
		sampler_create_info.anisotropyEnable        = true;
		sampler_create_info.maxAnisotropy           = physical_device_props.limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable           = false;
		sampler_create_info.compareOp               = vk::CompareOp::eAlways;
		sampler_create_info.minLod                  = 0.0f;
		sampler_create_info.maxLod                  = vk::LodClampNone;
		sampler_create_info.borderColor             = vk::BorderColor::eFloatOpaqueWhite;
		sampler_create_info.unnormalizedCoordinates = false;

		return (static_cast<vk::Device>(m_logicalDevice)).createSampler(sampler_create_info);
	}
}
