#include "vk_logical_device.hpp"

#include "vk_command_buffer.hpp"

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

namespace toaster::gpu
{
	VKLogicalDevice::VKLogicalDevice(VKPhysicalDevice *p_physical_device, const VKLogicalDeviceSpecInfo &p_spec_info) : m_physicalDevice(p_physical_device),
																														m_specInfo(p_spec_info)
	{
		const auto queue_family_props = m_physicalDevice->getVulkanPhysicalDevice().getQueueFamilyProperties();

		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			if (queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics && m_specInfo.usePresent
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

		LOG_TRACE("Graphics queue family index {}", m_queueFamilyIndices.graphics);
		LOG_TRACE("Transfer queue family index {}", m_queueFamilyIndices.transfer);
		LOG_TRACE("Compute queue family index {}", m_queueFamilyIndices.compute);

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

		m_pendingDeletions.resize(m_specInfo.maxFramesInFlight);
		m_pendingResourceUpdates.resize(m_specInfo.maxFramesInFlight);
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

	auto VKLogicalDevice::setCurrentFrameIndex(const uint32 p_index) -> void
	{
		TST_ASSERT_MSG(p_index < m_specInfo.maxFramesInFlight, "Index is out of bounds!");
		m_currentFrameIndex = p_index;
	}

	auto VKLogicalDevice::performGarbageCollection() -> void
	{
		if (!m_pendingResourceUpdates[m_currentFrameIndex].empty())
		{
			m_logicalDevice.waitIdle();
			while (!m_pendingResourceUpdates[m_currentFrameIndex].empty())
			{
				auto func{std::move(m_pendingResourceUpdates[m_currentFrameIndex].front())};
				m_pendingResourceUpdates[m_currentFrameIndex].pop_front();
				func();
			}
		}
		while (!m_pendingDeletions[m_currentFrameIndex].empty())
		{
			auto deleter{std::move(m_pendingDeletions[m_currentFrameIndex].front())};
			m_pendingDeletions[m_currentFrameIndex].pop_front();
			deleter();
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

	auto VKLogicalDevice::createBuffer(vk::DeviceSize    p_size, vk::BufferUsageFlags          p_usage_flags, vk::MemoryPropertyFlags p_memory_properties,
									   vk::raii::Buffer &p_out_buffer, vk::raii::DeviceMemory &p_out_memory) -> void
	{
		vk::BufferCreateInfo buffer_create_info{};
		buffer_create_info.size        = p_size;
		buffer_create_info.usage       = p_usage_flags;
		buffer_create_info.sharingMode = vk::SharingMode::eConcurrent;

		buffer_create_info.queueFamilyIndexCount = 2;
		uint32 qfi[]                             = {m_queueFamilyIndices.graphics, m_queueFamilyIndices.transfer};
		buffer_create_info.pQueueFamilyIndices   = qfi;

		p_out_buffer = {m_logicalDevice, buffer_create_info};

		vk::MemoryRequirements memory_requirements = p_out_buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.memoryTypeIndex = m_physicalDevice->findMemoryType(memory_requirements.memoryTypeBits, p_memory_properties);
		memory_allocate_info.allocationSize  = memory_requirements.size;

		p_out_memory = {m_logicalDevice, memory_allocate_info};

		p_out_buffer.bindMemory(p_out_memory, 0u);
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
										  uint32     p_mip_levels) -> vk::raii::ImageView
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

	auto VKLogicalDevice::createSampler() -> vk::raii::Sampler
	{
		const auto physical_device_props = m_physicalDevice->getVulkanPhysicalDevice().getProperties();

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.magFilter               = vk::Filter::eLinear;
		sampler_create_info.minFilter               = vk::Filter::eLinear;
		sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.mipLodBias              = 0.0f;
		sampler_create_info.anisotropyEnable        = true;
		sampler_create_info.maxAnisotropy           = physical_device_props.limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable           = false;
		sampler_create_info.compareOp               = vk::CompareOp::eAlways;
		sampler_create_info.minLod                  = 0.0f;
		sampler_create_info.maxLod                  = vk::LodClampNone;
		sampler_create_info.borderColor             = vk::BorderColor::eFloatCustomEXT;
		sampler_create_info.unnormalizedCoordinates = false;

		// Ts is purely aesthetic
		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;

		sampler_create_info.pNext = &border_colour_create_info;

		return {m_logicalDevice, sampler_create_info};
	}

	auto VKLogicalDevice::copyBuffer(vk::raii::Buffer &p_src_buffer, vk::raii::Buffer &p_dst_buffer, vk::DeviceSize p_size) -> void
	{
		vk::BufferCopy2 buffer_copy{};
		buffer_copy.size      = static_cast<uint32>(p_size);
		buffer_copy.srcOffset = 0;
		buffer_copy.dstOffset = 0;

		vk::CopyBufferInfo2 copy_info{};
		copy_info.srcBuffer   = p_src_buffer;
		copy_info.dstBuffer   = p_dst_buffer;
		copy_info.regionCount = 1;
		copy_info.pRegions    = &buffer_copy;

		VKCommandBuffer cmd{this, vk::QueueFlagBits::eTransfer};
		cmd.begin();
		cmd.getVulkanCommandBuffer().copyBuffer2(copy_info);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKLogicalDevice::copyBufferToImage(vk::raii::Buffer &p_src_buffer, vk::raii::Image &p_dst_image, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void
	{
		vk::BufferImageCopy2 image_copy{};
		image_copy.bufferOffset      = 0;
		image_copy.bufferRowLength   = 0;
		image_copy.bufferImageHeight = 0;
		image_copy.imageOffset       = vk::Offset3D{0, 0, 0};
		image_copy.imageExtent       = p_image_extent;
		image_copy.imageSubresource  = {vk::ImageAspectFlagBits::eColor, 0, 0, p_layer_count};

		vk::CopyBufferToImageInfo2 buffer_image_copy{};
		buffer_image_copy.srcBuffer      = p_src_buffer;
		buffer_image_copy.dstImage       = p_dst_image;
		buffer_image_copy.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
		buffer_image_copy.regionCount    = 1;
		buffer_image_copy.pRegions       = &image_copy;

		VKCommandBuffer cmd{this, vk::QueueFlagBits::eTransfer};
		cmd.begin();
		cmd.getVulkanCommandBuffer().copyBufferToImage2(buffer_image_copy);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKLogicalDevice::transitionImageLayout(vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
												vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
												uint32           p_layer_count, uint32 p_mip_levels, vk::ImageAspectFlags p_aspect_flags) -> void
	{
		vk::ImageMemoryBarrier2 image_memory_barrier{};
		image_memory_barrier.oldLayout           = p_old_layout;
		image_memory_barrier.newLayout           = p_new_layout;
		image_memory_barrier.srcAccessMask       = p_src_access_mask;
		image_memory_barrier.dstAccessMask       = p_dst_access_mask;
		image_memory_barrier.srcStageMask        = p_src_stage_mask;
		image_memory_barrier.dstStageMask        = p_dst_stage_mask;
		image_memory_barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.image               = p_image;
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, p_mip_levels, 0, p_layer_count};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		VKCommandBuffer cmd{this, vk::QueueFlagBits::eGraphics};
		cmd.begin();
		cmd.getVulkanCommandBuffer().pipelineBarrier2(dependency_info);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKLogicalDevice::transitionImageLayout(vk::raii::CommandBuffer &p_cmd, vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
												vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
												vk::PipelineStageFlags2  p_dst_stage_mask, uint32 p_layer_count, uint32 p_mip_levels,
												vk::ImageAspectFlags     p_aspect_flags) -> void
	{
		vk::ImageMemoryBarrier2 image_memory_barrier{};
		image_memory_barrier.oldLayout           = p_old_layout;
		image_memory_barrier.newLayout           = p_new_layout;
		image_memory_barrier.srcAccessMask       = p_src_access_mask;
		image_memory_barrier.dstAccessMask       = p_dst_access_mask;
		image_memory_barrier.srcStageMask        = p_src_stage_mask;
		image_memory_barrier.dstStageMask        = p_dst_stage_mask;
		image_memory_barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.image               = p_image;
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, p_mip_levels, 0, p_layer_count};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		p_cmd.pipelineBarrier2(dependency_info);
	}

	auto VKLogicalDevice::transitionImageLayout(vk::Image &      p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
												vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
												uint32           p_layer_count, uint32 p_mip_levels, vk::ImageAspectFlags p_aspect_flags) -> void
	{
		vk::ImageMemoryBarrier2 image_memory_barrier{};
		image_memory_barrier.oldLayout           = p_old_layout;
		image_memory_barrier.newLayout           = p_new_layout;
		image_memory_barrier.srcAccessMask       = p_src_access_mask;
		image_memory_barrier.dstAccessMask       = p_dst_access_mask;
		image_memory_barrier.srcStageMask        = p_src_stage_mask;
		image_memory_barrier.dstStageMask        = p_dst_stage_mask;
		image_memory_barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.image               = p_image;
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, p_mip_levels, 0, p_layer_count};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		VKCommandBuffer cmd{this, vk::QueueFlagBits::eGraphics};
		cmd.begin();
		cmd.getVulkanCommandBuffer().pipelineBarrier2(dependency_info);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKLogicalDevice::transitionImageLayout(vk::raii::CommandBuffer &p_cmd, vk::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
												vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
												vk::PipelineStageFlags2  p_dst_stage_mask, uint32 p_layer_count, uint32 p_mip_levels,
												vk::ImageAspectFlags     p_aspect_flags) -> void
	{
		vk::ImageMemoryBarrier2 image_memory_barrier{};
		image_memory_barrier.oldLayout           = p_old_layout;
		image_memory_barrier.newLayout           = p_new_layout;
		image_memory_barrier.srcAccessMask       = p_src_access_mask;
		image_memory_barrier.dstAccessMask       = p_dst_access_mask;
		image_memory_barrier.srcStageMask        = p_src_stage_mask;
		image_memory_barrier.dstStageMask        = p_dst_stage_mask;
		image_memory_barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.image               = p_image;
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, p_mip_levels, 0, p_layer_count};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		p_cmd.pipelineBarrier2(dependency_info);
	}

	auto VKLogicalDevice::generateMipmaps(vk::raii::Image &p_src_image, const ImageExtent &p_image_extent, uint32 p_mip_levels) -> void
	{
		vk::ImageMemoryBarrier2 memory_barrier{};
		memory_barrier.image                           = p_src_image;
		memory_barrier.oldLayout                       = vk::ImageLayout::eTransferDstOptimal;
		memory_barrier.newLayout                       = vk::ImageLayout::eTransferSrcOptimal;
		memory_barrier.srcAccessMask                   = vk::AccessFlagBits2::eTransferWrite;
		memory_barrier.dstAccessMask                   = vk::AccessFlagBits2::eTransferRead;
		memory_barrier.srcQueueFamilyIndex             = vk::QueueFamilyIgnored;
		memory_barrier.dstQueueFamilyIndex             = vk::QueueFamilyIgnored;
		memory_barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
		memory_barrier.subresourceRange.baseArrayLayer = 0;
		memory_barrier.subresourceRange.baseMipLevel   = 0;
		memory_barrier.subresourceRange.layerCount     = 1;
		memory_barrier.subresourceRange.levelCount     = 1;

		int32 mip_width{static_cast<int32>(p_image_extent.width)};
		int32 mip_height{static_cast<int32>(p_image_extent.height)};

		VKCommandBuffer cmd{this, vk::QueueFlagBits::eGraphics};
		cmd.begin();

		for (uint32 i{1u}; i < p_mip_levels; ++i)
		{
			memory_barrier.subresourceRange.baseMipLevel = i - 1;
			memory_barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
			memory_barrier.newLayout                     = vk::ImageLayout::eTransferSrcOptimal;
			memory_barrier.srcAccessMask                 = vk::AccessFlagBits2::eTransferWrite;
			memory_barrier.dstAccessMask                 = vk::AccessFlagBits2::eTransferRead;

			{
				memory_barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
				memory_barrier.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;

				vk::DependencyInfo dependency_info{};
				dependency_info.imageMemoryBarrierCount = 1;
				dependency_info.pImageMemoryBarriers    = &memory_barrier;
				cmd.getVulkanCommandBuffer().pipelineBarrier2(dependency_info);
			}

			std::array<vk::Offset3D, 2> src_offsets;
			std::array<vk::Offset3D, 2> dst_offsets;

			src_offsets[0] = vk::Offset3D{0, 0, 0};
			src_offsets[1] = vk::Offset3D{mip_width, mip_height, 1};

			dst_offsets[0] = vk::Offset3D{0, 0, 0};
			dst_offsets[1] = vk::Offset3D{mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1};

			vk::ImageBlit2 image_blit{};
			image_blit.srcOffsets     = src_offsets;
			image_blit.dstOffsets     = dst_offsets;
			image_blit.srcSubresource = vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i - 1, 0, 1};
			image_blit.dstSubresource = vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i, 0, 1};

			vk::BlitImageInfo2 blit_info{};
			blit_info.srcImage       = p_src_image;
			blit_info.dstImage       = p_src_image;
			blit_info.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
			blit_info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
			blit_info.regionCount    = 1;
			blit_info.pRegions       = &image_blit;
			blit_info.filter         = vk::Filter::eLinear;
			cmd.getVulkanCommandBuffer().blitImage2(blit_info);

			memory_barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
			memory_barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
			memory_barrier.srcAccessMask = vk::AccessFlagBits2::eTransferRead;
			memory_barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

			{
				memory_barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
				memory_barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;

				vk::DependencyInfo dependency_info{};
				dependency_info.imageMemoryBarrierCount = 1;
				dependency_info.pImageMemoryBarriers    = &memory_barrier;
				cmd.getVulkanCommandBuffer().pipelineBarrier2(dependency_info);
			}

			if (mip_width > 1)
				mip_width /= 2;
			if (mip_height > 1)
				mip_height /= 2;
		}

		memory_barrier.subresourceRange.baseMipLevel = p_mip_levels - 1;
		memory_barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
		memory_barrier.newLayout                     = vk::ImageLayout::eShaderReadOnlyOptimal;
		memory_barrier.srcAccessMask                 = vk::AccessFlagBits2::eTransferWrite;
		memory_barrier.dstAccessMask                 = vk::AccessFlagBits2::eShaderRead;

		{
			memory_barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
			memory_barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;

			vk::DependencyInfo dependency_info{};
			dependency_info.imageMemoryBarrierCount = 1;
			dependency_info.pImageMemoryBarriers    = &memory_barrier;
			cmd.getVulkanCommandBuffer().pipelineBarrier2(dependency_info);
		}

		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKLogicalDevice::hasStencilComponent(vk::Format p_format) const -> bool
	{
		return p_format == vk::Format::eD32SfloatS8Uint || p_format == vk::Format::eD24UnormS8Uint;
	}

	auto VKLogicalDevice::isDepthFormat(vk::Format p_format) const -> bool
	{
		return p_format == vk::Format::eD16Unorm || p_format == vk::Format::eD16UnormS8Uint || p_format == vk::Format::eD24UnormS8Uint || p_format ==
			   vk::Format::eD32Sfloat || p_format == vk::Format::eD32SfloatS8Uint;
	}
}
