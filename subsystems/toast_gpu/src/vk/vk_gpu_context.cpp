#include "toast_gpu/vk/vk_gpu_context.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_descriptor_heap.hpp"

namespace toaster::gpu
{
	VKGPUContext::VKGPUContext(const GPUContextSpecInfo &p_spec_info) : m_specInfo(p_spec_info)
	{
		bool use_present{p_spec_info.instanceExtensions.contains(VK_KHR_SURFACE_EXTENSION_NAME)};

		VKInstanceSpecInfo vk_instance_spec_info{};
		vk_instance_spec_info.appName            = "Toaster-2.0 -> Vulkan";
		vk_instance_spec_info.requiredExtensions = p_spec_info.instanceExtensions;
		m_backendInstance                        = new VKInstance{vk_instance_spec_info};

		std::unordered_set<String> required_device_extensions{
			vk::KHRDynamicRenderingExtensionName,
			vk::KHRMaintenance6ExtensionName,
			vk::KHRLoadStoreOpNoneExtensionName,
			vk::EXTShaderObjectExtensionName,
			vk::KHRBufferDeviceAddressExtensionName,
			vk::EXTDescriptorHeapExtensionName,
			vk::KHRShaderUntypedPointersExtensionName,
			vk::EXTMeshShaderExtensionName
		};
		if (use_present)
			required_device_extensions.insert(vk::KHRSwapchainExtensionName);

		VKPhysicalDeviceSpecInfo vk_physical_device_spec_info{};
		vk_physical_device_spec_info.requiredExtensions = required_device_extensions;

		m_physicalDevice = new VKPhysicalDevice{*m_backendInstance, vk_physical_device_spec_info};

		VKLogicalDeviceSpecInfo vk_logical_device_spec_info{};
		vk_logical_device_spec_info.requiredExtensions   = required_device_extensions;
		vk_logical_device_spec_info.printShaderDebugInfo = p_spec_info.printDebugInfo;
		auto features{VKLogicalDeviceSpecInfo::getDefaultFeatures()};

		vk_logical_device_spec_info.pNext = features.get<vk::PhysicalDeviceFeatures2>();

		m_logicalDevice = new VKLogicalDevice{*m_physicalDevice, vk_logical_device_spec_info};

		m_pendingDeletionCommandQueues.resize(p_spec_info.maxFramesInFlight);

		m_descriptorHeap = new VKDescriptorHeap{*this};
	}

	VKGPUContext::~VKGPUContext()
	{
		delete m_descriptorHeap;

		// Clean up any remaining objects
		performGarbageCollection();

		delete m_logicalDevice;
		delete m_physicalDevice;
		delete m_backendInstance;
	}

	auto VKGPUContext::getSpecInfo() const -> const GPUContextSpecInfo &
	{
		return m_specInfo;
	}

	auto VKGPUContext::getBackendInstance() const -> VKInstance *
	{
		return m_backendInstance;
	}

	auto VKGPUContext::getPhysicalDevice() const -> VKPhysicalDevice *
	{
		return m_physicalDevice;
	}

	auto VKGPUContext::getLogicalDevice() const -> VKLogicalDevice *
	{
		return m_logicalDevice;
	}

	auto VKGPUContext::getDescriptorHeap() const -> VKDescriptorHeap *
	{
		return m_descriptorHeap;
	}

	auto VKGPUContext::getCurrentFrameIndex() const -> uint32
	{
		return m_currentFrameIndex;
	}

	auto VKGPUContext::getCurrentCommandBuffer() const -> VKCommandBuffer *
	{
		return m_currentCommandBuffer;
	}

	auto VKGPUContext::setCurrentFrameIndex(uint32 p_index) -> void
	{
		TST_ASSERT(p_index < m_specInfo.maxFramesInFlight);
		m_currentFrameIndex = p_index;
	}

	auto VKGPUContext::setCurrentCommandBuffer(VKCommandBuffer *p_cmd) -> void
	{
		m_currentCommandBuffer = p_cmd;
	}

	auto VKGPUContext::performGarbageCollection() -> void
	{
		// Very simple!!!
		m_pendingDeletionCommandQueues[m_currentFrameIndex].execute();
	}

	auto VKGPUContext::bindDescriptorHeap(VKCommandBuffer *p_command_buffer) const -> void
	{
		VKCommandBuffer *cmd{p_command_buffer ? p_command_buffer : m_currentCommandBuffer};

		const auto &heap_props{m_descriptorHeap->getHeapProperties()};

		// bind the resource heap
		{
			const VKBuffer &resource_heap{m_descriptorHeap->getResourceHeap()};

			vk::BindHeapInfoEXT resource_heap_bind_info{};
			resource_heap_bind_info.heapRange           = resource_heap.getDeviceAddressRange();
			resource_heap_bind_info.reservedRangeOffset = resource_heap.getSize() - heap_props.minResourceHeapReservedRange;
			resource_heap_bind_info.reservedRangeSize   = heap_props.minResourceHeapReservedRange;

			cmd->getVulkanCommandBuffer().bindResourceHeapEXT(resource_heap_bind_info);
		}

		// bind the sampler heap
		{
			const VKBuffer &sampler_heap{m_descriptorHeap->getSamplerHeap()};

			vk::BindHeapInfoEXT sampler_heap_bind_info{};
			sampler_heap_bind_info.heapRange           = sampler_heap.getDeviceAddressRange();
			sampler_heap_bind_info.reservedRangeOffset = sampler_heap.getSize() - heap_props.minSamplerHeapReservedRange;
			sampler_heap_bind_info.reservedRangeSize   = heap_props.minSamplerHeapReservedRange;

			cmd->getVulkanCommandBuffer().bindSamplerHeapEXT(sampler_heap_bind_info);
		}
	}

	auto VKGPUContext::copyBuffer(vk::raii::Buffer &p_src_buffer, vk::raii::Buffer &p_dst_buffer, vk::DeviceSize p_size) -> void
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

		VKCommandBuffer cmd{*this, vk::QueueFlagBits::eTransfer};
		cmd.begin();
		cmd.getVulkanCommandBuffer().copyBuffer2(copy_info);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKGPUContext::copyBuffer(const vk::Buffer &p_src_buffer, const vk::Buffer &p_dst_buffer, vk::DeviceSize p_size) -> void
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

		VKCommandBuffer cmd{*this, vk::QueueFlagBits::eTransfer};
		cmd.begin();
		cmd.getVulkanCommandBuffer().copyBuffer2(copy_info);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKGPUContext::copyBufferToImage(vk::raii::Buffer &p_src_buffer, vk::raii::Image &p_dst_image, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void
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

		VKCommandBuffer cmd{*this, vk::QueueFlagBits::eTransfer};
		cmd.begin();
		cmd.getVulkanCommandBuffer().copyBufferToImage2(buffer_image_copy);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKGPUContext::copyBufferToImage(const vk::Buffer &p_src_buffer, const vk::Image &p_dst_image, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void
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

		VKCommandBuffer cmd{*this, vk::QueueFlagBits::eTransfer};
		cmd.begin();
		cmd.getVulkanCommandBuffer().copyBufferToImage2(buffer_image_copy);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKGPUContext::copyImageToBuffer(vk::raii::Image &p_src_image, vk::raii::Buffer &p_dst_buffer, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void
	{
		vk::BufferImageCopy2 image_copy{};
		image_copy.bufferOffset      = 0;
		image_copy.bufferRowLength   = 0;
		image_copy.bufferImageHeight = 0;
		image_copy.imageOffset       = vk::Offset3D{0, 0, 0};
		image_copy.imageExtent       = p_image_extent;
		image_copy.imageSubresource  = {vk::ImageAspectFlagBits::eColor, 0, 0, p_layer_count};

		vk::CopyImageToBufferInfo2 image_buffer_copy{};
		image_buffer_copy.srcImage       = p_src_image;
		image_buffer_copy.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
		image_buffer_copy.dstBuffer      = p_dst_buffer;
		image_buffer_copy.regionCount    = 1;
		image_buffer_copy.pRegions       = &image_copy;

		VKCommandBuffer cmd{*this, vk::QueueFlagBits::eTransfer};
		cmd.begin();
		cmd.getVulkanCommandBuffer().copyImageToBuffer2(image_buffer_copy);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKGPUContext::copyImageToBuffer(const vk::Image &p_src_image, const vk::Buffer &p_dst_buffer, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void
	{
		vk::BufferImageCopy2 image_copy{};
		image_copy.bufferOffset      = 0;
		image_copy.bufferRowLength   = 0;
		image_copy.bufferImageHeight = 0;
		image_copy.imageOffset       = vk::Offset3D{0, 0, 0};
		image_copy.imageExtent       = p_image_extent;
		image_copy.imageSubresource  = {vk::ImageAspectFlagBits::eColor, 0, 0, p_layer_count};

		vk::CopyImageToBufferInfo2 image_buffer_copy{};
		image_buffer_copy.srcImage       = p_src_image;
		image_buffer_copy.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
		image_buffer_copy.dstBuffer      = p_dst_buffer;
		image_buffer_copy.regionCount    = 1;
		image_buffer_copy.pRegions       = &image_copy;

		VKCommandBuffer cmd{*this, vk::QueueFlagBits::eTransfer};
		cmd.begin();
		cmd.getVulkanCommandBuffer().copyImageToBuffer2(image_buffer_copy);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKGPUContext::transitionImageLayout(vk::raii::Image & p_image, const ImageLayoutInfo &p_src_layout_info, const ImageLayoutInfo &p_dst_layout_info,
											 uint32            p_layer_count, uint32           p_mip_levels, vk::ImageAspectFlags        p_aspect_flags,
											 vk::CommandBuffer p_override_command_buffer) -> void
	{
		vk::ImageMemoryBarrier2 image_memory_barrier{};
		image_memory_barrier.oldLayout           = p_src_layout_info.layout;
		image_memory_barrier.newLayout           = p_dst_layout_info.layout;
		image_memory_barrier.srcAccessMask       = p_src_layout_info.accessMask;
		image_memory_barrier.dstAccessMask       = p_dst_layout_info.accessMask;
		image_memory_barrier.srcStageMask        = p_src_layout_info.stageMask;
		image_memory_barrier.dstStageMask        = p_dst_layout_info.stageMask;
		image_memory_barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.image               = p_image;
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, p_mip_levels, 0, p_layer_count};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		if (!p_override_command_buffer)
		{
			VKCommandBuffer cmd{*this, vk::QueueFlagBits::eGraphics};
			cmd.begin();
			cmd.getVulkanCommandBuffer().pipelineBarrier2(dependency_info);
			cmd.end();
			cmd.submit();
			cmd.waitForFence();
		}
		else
		{
			p_override_command_buffer.pipelineBarrier2(dependency_info);
		}
	}

	auto VKGPUContext::transitionImageLayout(vk::Image &p_image, const ImageLayoutInfo &p_src_layout_info, const ImageLayoutInfo &p_dst_layout_info, uint32 p_layer_count,
											 uint32     p_mip_levels, vk::ImageAspectFlags p_aspect_flags, vk::CommandBuffer p_override_command_buffer) -> void
	{
		vk::ImageMemoryBarrier2 image_memory_barrier{};
		image_memory_barrier.oldLayout           = p_src_layout_info.layout;
		image_memory_barrier.newLayout           = p_dst_layout_info.layout;
		image_memory_barrier.srcAccessMask       = p_src_layout_info.accessMask;
		image_memory_barrier.dstAccessMask       = p_dst_layout_info.accessMask;
		image_memory_barrier.srcStageMask        = p_src_layout_info.stageMask;
		image_memory_barrier.dstStageMask        = p_dst_layout_info.stageMask;
		image_memory_barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.image               = p_image;
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, p_mip_levels, 0, p_layer_count};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		if (!p_override_command_buffer)
		{
			VKCommandBuffer cmd{*this, vk::QueueFlagBits::eGraphics};
			cmd.begin();
			cmd.getVulkanCommandBuffer().pipelineBarrier2(dependency_info);
			cmd.end();
			cmd.submit();
			cmd.waitForFence();
		}
		else
		{
			p_override_command_buffer.pipelineBarrier2(dependency_info);
		}
	}

	auto VKGPUContext::transitionImageLayout(vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
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

		VKCommandBuffer cmd{*this, vk::QueueFlagBits::eGraphics};
		cmd.begin();
		cmd.getVulkanCommandBuffer().pipelineBarrier2(dependency_info);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKGPUContext::transitionImageLayout(vk::raii::CommandBuffer &p_cmd, vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
											 vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
											 vk::PipelineStageFlags2  p_dst_stage_mask, uint32 p_layer_count, uint32 p_mip_levels,
											 vk::ImageAspectFlags     p_aspect_flags) const -> void
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

	auto VKGPUContext::transitionImageLayout(vk::Image &      p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
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

		VKCommandBuffer cmd{*this, vk::QueueFlagBits::eGraphics};
		cmd.begin();
		cmd.getVulkanCommandBuffer().pipelineBarrier2(dependency_info);
		cmd.end();
		cmd.submit();
		cmd.waitForFence();
	}

	auto VKGPUContext::transitionImageLayout(vk::raii::CommandBuffer &p_cmd, vk::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
											 vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
											 vk::PipelineStageFlags2  p_dst_stage_mask, uint32 p_layer_count, uint32 p_mip_levels,
											 vk::ImageAspectFlags     p_aspect_flags) const -> void
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

	auto VKGPUContext::generateMipmaps(vk::raii::Image &p_src_image, const ImageExtent &p_image_extent, uint32 p_mip_levels) -> void
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

		VKCommandBuffer cmd{*this, vk::QueueFlagBits::eGraphics};
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

	auto VKGPUContext::generateMipmaps(vk::Image &p_src_image, const ImageExtent &p_image_extent, uint32 p_mip_levels) -> void
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

		VKCommandBuffer cmd{*this, vk::QueueFlagBits::eGraphics};
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
}
