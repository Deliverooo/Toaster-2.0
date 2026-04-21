#include "vk_gpu_context.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"
#include "toast_lib/util_defines.hpp"

#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	VKGPUContext::VKGPUContext(VKLogicalDevice *p_logical_device, const VKGPUContextSpecInfo &p_spec_info) : m_logicalDevice(p_logical_device), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(m_logicalDevice, "Logical device is null");
	}

	auto VKGPUContext::setCurrentFrameIndex(uint32 p_index) -> void
	{
		m_currentFrameIndex = p_index;
	}

	auto VKGPUContext::performGarbageCollection() -> void
	{
		while (!m_pendingDeletions[m_currentFrameIndex].empty())
		{
			auto deleter{std::move(m_pendingDeletions[m_currentFrameIndex].front())};
			m_pendingDeletions[m_currentFrameIndex].pop_front();
			deleter();
		}

		while (!m_pendingResourceUpdates[m_currentFrameIndex].empty())
		{
			auto func{std::move(m_pendingResourceUpdates[m_currentFrameIndex].front())};
			m_pendingResourceUpdates[m_currentFrameIndex].pop_front();
			func();
		}
	}

	auto VKGPUContext::getSpecInfo() const -> const VKGPUContextSpecInfo &
	{
		return m_specInfo;
	}

	auto VKGPUContext::getInstance() const -> VKInstance *
	{
		return m_logicalDevice->getPhysicalDevice()->getInstance();
	}

	auto VKGPUContext::getPhysicalDevice() -> VKPhysicalDevice *
	{
		return m_logicalDevice->getPhysicalDevice();
	}

	auto VKGPUContext::getLogicalDevice() -> VKLogicalDevice *
	{
		return m_logicalDevice;
	}

	auto VKGPUContext::transitionImageLayout(vk::raii::CommandBuffer &p_command_buffer, vk::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
											 vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
											 vk::PipelineStageFlags2  p_dst_stage_mask, vk::ImageAspectFlags p_aspect_flags) -> void
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
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, 1, 0, 1};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		p_command_buffer.pipelineBarrier2(dependency_info);
	}

	auto VKGPUContext::transitionImageLayout(vk::raii::CommandBuffer &p_command_buffer, vk::raii::Image &       p_image, vk::ImageLayout            p_old_layout,
											 vk::ImageLayout          p_new_layout, vk::AccessFlags2            p_src_access_mask, vk::AccessFlags2 p_dst_access_mask,
											 vk::PipelineStageFlags2  p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
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
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, 1, 0, 1};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		p_command_buffer.pipelineBarrier2(dependency_info);
	}

	auto VKGPUContext::createShaderModule(const std::vector<uint8> &p_code) -> vk::raii::ShaderModule
	{
		vk::ShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.codeSize = p_code.size();
		shader_module_create_info.pCode    = reinterpret_cast<const uint32 *>(p_code.data());

		return {m_logicalDevice->getVulkanLogicalDevice(), shader_module_create_info};
	}

	auto VKGPUContext::createShaderModule(const std::vector<uint32> &p_code) -> vk::raii::ShaderModule
	{
		vk::ShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.codeSize = p_code.size() * sizeof(uint32);
		shader_module_create_info.pCode    = p_code.data();

		return {m_logicalDevice->getVulkanLogicalDevice(), shader_module_create_info};
	}

	auto VKGPUContext::findMemoryType(uint32 p_type_filter, vk::MemoryPropertyFlags p_properties) const -> uint32
	{
		vk::PhysicalDeviceMemoryProperties memory_properties = m_logicalDevice->getPhysicalDevice()->getVulkanPhysicalDevice().getMemoryProperties();
		for (uint32 i{0u}; i < memory_properties.memoryTypeCount; i++)
		{
			if ((p_type_filter & BIT(i)) && (memory_properties.memoryTypes[i].propertyFlags & p_properties) == p_properties)
			{
				return i;
			}
		}
		TST_ASSERT_MSG(false, "Failed to find a matching memory type!");
		return UINT32_MAX;
	}

	auto VKGPUContext::createBuffer(vk::DeviceSize    p_size, vk::BufferUsageFlags          p_usage_flags, vk::MemoryPropertyFlags p_memory_properties,
									vk::raii::Buffer &p_out_buffer, vk::raii::DeviceMemory &p_out_memory) const -> void
	{
		vk::BufferCreateInfo buffer_create_info{};
		buffer_create_info.size        = p_size;
		buffer_create_info.usage       = p_usage_flags;
		buffer_create_info.sharingMode = vk::SharingMode::eConcurrent;

		auto queue_family_indices{m_logicalDevice->getQueueFamilyIndices()};

		buffer_create_info.queueFamilyIndexCount = 2;
		uint32 qfi[]                             = {queue_family_indices.graphics, queue_family_indices.transfer};
		buffer_create_info.pQueueFamilyIndices   = qfi;

		p_out_buffer = {m_logicalDevice->getVulkanLogicalDevice(), buffer_create_info};

		vk::MemoryRequirements memory_requirements = p_out_buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, p_memory_properties);
		memory_allocate_info.allocationSize  = memory_requirements.size;

		p_out_memory = {m_logicalDevice->getVulkanLogicalDevice(), memory_allocate_info};

		p_out_buffer.bindMemory(p_out_memory, 0u);
	}

	auto VKGPUContext::copyBuffer(vk::raii::Buffer &p_src_buffer, vk::raii::Buffer &p_dst_buffer, vk::DeviceSize p_size) const -> void
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

		vk::raii::CommandBuffer cmd = beginSingleTimeCommandsTransfer();
		cmd.copyBuffer2(copy_info);
		endSingleTimeCommandsTransfer(cmd);
	}

	auto VKGPUContext::createImage(uint32           p_width, uint32 p_height, uint32 p_mip_levels, vk::SampleCountFlagBits p_sample_count, vk::Format p_format,
								   vk::ImageTiling  p_image_tiling, vk::ImageUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties,
								   vk::raii::Image &p_out_image, vk::raii::DeviceMemory &p_out_memory) const -> void
	{
		vk::ImageCreateInfo image_create_info{};
		image_create_info.extent.width  = p_width;
		image_create_info.extent.height = p_height;
		image_create_info.extent.depth  = 1;
		image_create_info.mipLevels     = p_mip_levels;
		image_create_info.arrayLayers   = 1;
		image_create_info.imageType     = vk::ImageType::e2D;
		image_create_info.samples       = p_sample_count;
		image_create_info.sharingMode   = vk::SharingMode::eConcurrent;
		image_create_info.tiling        = p_image_tiling;
		image_create_info.initialLayout = vk::ImageLayout::eUndefined;
		image_create_info.usage         = p_usage_flags;
		image_create_info.format        = p_format;

		auto queue_family_indices{m_logicalDevice->getQueueFamilyIndices()};

		image_create_info.queueFamilyIndexCount = 2;
		uint32 qfi[]                            = {queue_family_indices.graphics, queue_family_indices.transfer};
		image_create_info.pQueueFamilyIndices   = qfi;

		p_out_image                                = {m_logicalDevice->getVulkanLogicalDevice(), image_create_info};
		vk::MemoryRequirements memory_requirements = p_out_image.getMemoryRequirements();
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.allocationSize  = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, p_memory_properties);

		p_out_memory = {m_logicalDevice->getVulkanLogicalDevice(), memory_allocate_info};

		p_out_image.bindMemory(p_out_memory, 0u);
	}

	auto VKGPUContext::transitionImageLayout(vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
											 vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
											 uint32           p_mip_levels, vk::ImageAspectFlags p_aspect_flags) const -> void
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
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, p_mip_levels, 0, 1};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		vk::raii::CommandBuffer cmd = beginSingleTimeCommandsGraphics();
		cmd.pipelineBarrier2(dependency_info);
		endSingleTimeCommandsGraphics(cmd);
	}

	auto VKGPUContext::copyBufferToImage(vk::raii::Buffer &p_src_buffer, vk::raii::Image &p_dst_image, uint32 p_width, uint32 p_height) const -> void
	{
		vk::BufferImageCopy2 image_copy{};
		image_copy.bufferOffset      = 0;
		image_copy.bufferRowLength   = 0;
		image_copy.bufferImageHeight = 0;
		image_copy.imageOffset       = vk::Offset3D{0, 0, 0};
		image_copy.imageExtent       = vk::Extent3D{p_width, p_height, 1};
		image_copy.imageSubresource  = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};

		vk::CopyBufferToImageInfo2 buffer_image_copy{};
		buffer_image_copy.srcBuffer      = p_src_buffer;
		buffer_image_copy.dstImage       = p_dst_image;
		buffer_image_copy.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
		buffer_image_copy.regionCount    = 1;
		buffer_image_copy.pRegions       = &image_copy;

		vk::raii::CommandBuffer cmd = beginSingleTimeCommandsTransfer();
		cmd.copyBufferToImage2(buffer_image_copy);
		endSingleTimeCommandsTransfer(cmd);
	}

	auto VKGPUContext::createImageView(vk::raii::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags,
									   uint32           p_mip_levels) const -> vk::raii::ImageView
	{
		vk::ImageViewCreateInfo image_view_create_info{};
		image_view_create_info.viewType   = vk::ImageViewType::e2D;
		image_view_create_info.image      = p_src_image;
		image_view_create_info.components = {
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		};
		image_view_create_info.subresourceRange = vk::ImageSubresourceRange{p_aspect_flags, 0, p_mip_levels, 0, 1};
		image_view_create_info.format           = p_format;

		return {m_logicalDevice->getVulkanLogicalDevice(), image_view_create_info};
	}

	auto VKGPUContext::createImageView(vk::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags, uint32 p_mip_levels) const -> vk::raii::ImageView
	{
		vk::ImageViewCreateInfo image_view_create_info{};
		image_view_create_info.viewType   = vk::ImageViewType::e2D;
		image_view_create_info.image      = p_src_image;
		image_view_create_info.components = {
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		};
		image_view_create_info.subresourceRange = vk::ImageSubresourceRange{p_aspect_flags, 0, p_mip_levels, 0, 1};
		image_view_create_info.format           = p_format;

		return {m_logicalDevice->getVulkanLogicalDevice(), image_view_create_info};
	}

	auto VKGPUContext::generateMipmaps(vk::raii::Image &p_src_image, vk::Format p_format, uint32 p_width, uint32 p_height, uint32 p_mip_levels) const -> void
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

		int32 mip_width{static_cast<int32>(p_width)};
		int32 mip_height{static_cast<int32>(p_height)};

		vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.commandPool        = m_logicalDevice->getGraphicsCommandPool();
		command_buffer_allocate_info.level              = vk::CommandBufferLevel::ePrimary;

		vk::raii::CommandBuffer command_buffer{std::move(m_logicalDevice->getVulkanLogicalDevice().allocateCommandBuffers(command_buffer_allocate_info).front())};

		vk::CommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		command_buffer.begin(command_buffer_begin_info);

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
				command_buffer.pipelineBarrier2(dependency_info);
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
			command_buffer.blitImage2(blit_info);

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
				command_buffer.pipelineBarrier2(dependency_info);
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
			command_buffer.pipelineBarrier2(dependency_info);
		}

		command_buffer.end();

		vk::FenceCreateInfo fence_create_info{};
		vk::raii::Fence     wait_fence{m_logicalDevice->getVulkanLogicalDevice(), fence_create_info};

		vk::CommandBufferSubmitInfo command_buffer_submit_info{};
		command_buffer_submit_info.commandBuffer = *command_buffer;

		vk::SubmitInfo2 submit_info{};
		submit_info.commandBufferInfoCount = 1;
		submit_info.pCommandBufferInfos    = &command_buffer_submit_info;
		m_logicalDevice->getGraphicsQueue().submit2(submit_info, *wait_fence);

		vk::Result res = m_logicalDevice->getVulkanLogicalDevice().waitForFences(*wait_fence, true, UINT64_MAX);
		if (res != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to wait for Fence");
	}

	auto VKGPUContext::beginSingleTimeCommandsTransfer() const -> vk::raii::CommandBuffer
	{
		vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.commandPool        = m_logicalDevice->getTransferCommandPool();
		command_buffer_allocate_info.level              = vk::CommandBufferLevel::ePrimary;

		vk::raii::CommandBuffer command_buffer{std::move(m_logicalDevice->getVulkanLogicalDevice().allocateCommandBuffers(command_buffer_allocate_info).front())};

		vk::CommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		command_buffer.begin(command_buffer_begin_info);
		return command_buffer;
	}

	auto VKGPUContext::endSingleTimeCommandsTransfer(vk::raii::CommandBuffer &p_command_buffer) const -> void
	{
		p_command_buffer.end();

		vk::FenceCreateInfo fence_create_info{};
		vk::raii::Fence     wait_fence{m_logicalDevice->getVulkanLogicalDevice(), fence_create_info};

		vk::CommandBufferSubmitInfo command_buffer_submit_info{};
		command_buffer_submit_info.commandBuffer = *p_command_buffer;

		vk::SubmitInfo2 submit_info{};
		submit_info.commandBufferInfoCount = 1;
		submit_info.pCommandBufferInfos    = &command_buffer_submit_info;
		m_logicalDevice->getTransferQueue().submit2(submit_info, *wait_fence);

		vk::Result res = m_logicalDevice->getVulkanLogicalDevice().waitForFences(*wait_fence, true, UINT64_MAX);
		if (res != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to wait for Fence");
	}

	auto VKGPUContext::beginSingleTimeCommandsGraphics() const -> vk::raii::CommandBuffer
	{
		vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.commandPool        = m_logicalDevice->getGraphicsCommandPool();
		command_buffer_allocate_info.level              = vk::CommandBufferLevel::ePrimary;

		vk::raii::CommandBuffer command_buffer{std::move(m_logicalDevice->getVulkanLogicalDevice().allocateCommandBuffers(command_buffer_allocate_info).front())};

		vk::CommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		command_buffer.begin(command_buffer_begin_info);
		return command_buffer;
	}

	auto VKGPUContext::endSingleTimeCommandsGraphics(vk::raii::CommandBuffer &p_command_buffer) const -> void
	{
		p_command_buffer.end();

		vk::FenceCreateInfo fence_create_info{};
		vk::raii::Fence     wait_fence{m_logicalDevice->getVulkanLogicalDevice(), fence_create_info};

		vk::CommandBufferSubmitInfo command_buffer_submit_info{};
		command_buffer_submit_info.commandBuffer = *p_command_buffer;

		vk::SubmitInfo2 submit_info{};
		submit_info.commandBufferInfoCount = 1;
		submit_info.pCommandBufferInfos    = &command_buffer_submit_info;
		m_logicalDevice->getGraphicsQueue().submit2(submit_info, *wait_fence);

		vk::Result res = m_logicalDevice->getVulkanLogicalDevice().waitForFences(*wait_fence, true, UINT64_MAX);
		if (res != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to wait for Fence");
	}

	auto VKGPUContext::hasStencilComponent(const vk::Format p_format) const -> bool
	{
		return p_format == vk::Format::eD32SfloatS8Uint || p_format == vk::Format::eD24UnormS8Uint;
	}

	auto VKGPUContext::isDepthFormat(const vk::Format p_format) const -> bool
	{
		return p_format == vk::Format::eD16Unorm || p_format == vk::Format::eD16UnormS8Uint || p_format == vk::Format::eD24UnormS8Uint || p_format ==
			   vk::Format::eD32Sfloat || p_format == vk::Format::eD32SfloatS8Uint;
	}
}
