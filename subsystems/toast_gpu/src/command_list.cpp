#include "toast_gpu/command_list.hpp"

namespace toaster::gpu
{
	CommandList::CommandList(Device &p_device, vk::CommandBuffer p_cmd) : m_device(&p_device), m_cmd(p_cmd)
	{
	}

	CommandList::~CommandList()
	{
		// if (m_cmd)
		// m_device->m_device->getDevice().freeCommandBuffers(m_device->m_device->getGraphicsCommandPool(), {m_cmd});
	}

	CommandList::CommandList(const CommandList &p_other) : m_device(p_other.m_device), m_cmd(p_other.m_cmd)
	{
	}

	CommandList::CommandList(CommandList &&p_other) noexcept : m_device(p_other.m_device), m_cmd(p_other.m_cmd)
	{
		p_other.m_device = nullptr;
		p_other.m_cmd    = nullptr;
	}

	CommandList &CommandList::operator=(const CommandList &p_other)
	{
		if (this != &p_other)
		{
			m_device = p_other.m_device;
			m_cmd    = p_other.m_cmd;
		}
		return *this;
	}

	CommandList &CommandList::operator=(CommandList &&p_other) noexcept
	{
		m_device = p_other.m_device;
		m_cmd    = p_other.m_cmd;

		p_other.m_device = nullptr;
		p_other.m_cmd    = nullptr;

		return *this;
	}

	auto CommandList::reset() -> void
	{
		m_cmd.reset();
	}

	auto CommandList::begin() -> void
	{
		m_cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
	}

	auto CommandList::end() -> void
	{
		m_cmd.end();
	}

	auto CommandList::bindResourceHeap(const ResourceDescriptorHeap &p_resource_heap) -> void
	{
		m_cmd.bindResourceHeapEXT(p_resource_heap.getBindInfo(), FunctionDispatcher::get());
	}

	auto CommandList::bindSamplerHeap(const SamplerDescriptorHeap &p_sampler_heap) -> void
	{
		m_cmd.bindSamplerHeapEXT(p_sampler_heap.getBindInfo(), FunctionDispatcher::get());
	}

	auto CommandList::copyBuffer(BufferHandle p_src_buffer, BufferHandle p_dst_buffer, uint64 p_size, uint64 p_src_offset, uint64 p_dst_offset) -> void
	{
		const BufferData *src_data{m_device->getBufferData(p_src_buffer)};
		BufferData *      dst_data{m_device->getBufferData(p_dst_buffer)};

		TST_ASSERT(src_data && dst_data);
		TST_ASSERT_MSG(src_data->usageFlags & vk::BufferUsageFlagBits::eTransferSrc, "Src buffer is not a transfer src");
		TST_ASSERT_MSG(dst_data->usageFlags & vk::BufferUsageFlagBits::eTransferDst, "Dst buffer is not a transfer dst");

		vk::BufferCopy2 copy_region{};
		copy_region.srcOffset = p_src_offset;
		copy_region.dstOffset = p_dst_offset;
		copy_region.size      = p_size;

		vk::CopyBufferInfo2 copy_buffer_info{};
		copy_buffer_info.srcBuffer = src_data->buffer;
		copy_buffer_info.dstBuffer = dst_data->buffer;
		copy_buffer_info.setRegions(copy_region);
		m_cmd.copyBuffer2(copy_buffer_info);
	}

	auto CommandList::copyBufferToTexture(BufferHandle p_src_buffer, TextureHandle p_dst_texture) -> void
	{
		const BufferData *src_data{m_device->getBufferData(p_src_buffer)};
		TextureData *     dst_data{m_device->getTextureData(p_dst_texture)};
		TST_ASSERT(src_data && dst_data);
		TST_ASSERT_MSG(src_data->usageFlags & vk::BufferUsageFlagBits::eTransferSrc, "Src buffer is not a transfer src");
		TST_ASSERT_MSG(dst_data->layout == vk::ImageLayout::eTransferDstOptimal, "Dst texture is not in the transfer dst layout");

		vk::BufferImageCopy2 buffer_image_copy{};
		buffer_image_copy.bufferOffset      = 0;
		buffer_image_copy.bufferRowLength   = 0;
		buffer_image_copy.bufferImageHeight = 0;
		buffer_image_copy.imageOffset       = vk::Offset3D{0, 0, 0};
		buffer_image_copy.imageExtent       = dst_data->extent;
		buffer_image_copy.imageSubresource  = {getImageAspectMask(dst_data->format), 0, 0, dst_data->layerCount};

		vk::CopyBufferToImageInfo2 copy_buffer_to_image_info{};
		copy_buffer_to_image_info.srcBuffer      = src_data->buffer;
		copy_buffer_to_image_info.dstImage       = dst_data->image;
		copy_buffer_to_image_info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
		copy_buffer_to_image_info.setRegions(buffer_image_copy);

		m_cmd.copyBufferToImage2(copy_buffer_to_image_info);
	}

	auto CommandList::transitionTextureLayout(TextureHandle p_texture, vk::ImageLayout p_dst_layout) -> void
	{
		TextureData *texture_data{m_device->getTextureData(p_texture)};
		TST_ASSERT(texture_data);

		TST_ASSERT(p_dst_layout != vk::ImageLayout::eUndefined);
		if (texture_data->layout == p_dst_layout) // If the image is already in the specified layout, there is no need to transition.
			return;

		vk::ImageMemoryBarrier2 image_memory_barrier{};
		image_memory_barrier.image     = texture_data->image;
		image_memory_barrier.oldLayout = texture_data->layout;
		image_memory_barrier.newLayout = p_dst_layout;

		getImageAccessFlagsAndStageMask(texture_data->layout, image_memory_barrier.srcAccessMask, image_memory_barrier.srcStageMask);
		getImageAccessFlagsAndStageMask(p_dst_layout, image_memory_barrier.dstAccessMask, image_memory_barrier.dstStageMask);

		image_memory_barrier.subresourceRange = vk::ImageSubresourceRange{
			getImageAspectMask(texture_data->format),
			0u,
			texture_data->mipLevels,
			0u,
			texture_data->layerCount
		};

		vk::DependencyInfo dependency_info{};
		dependency_info.setImageMemoryBarriers(image_memory_barrier);

		m_cmd.pipelineBarrier2(dependency_info);

		texture_data->layout = p_dst_layout; // Maybe I shouldn't update the layout immediately, but it doesn't matter
	}
}
