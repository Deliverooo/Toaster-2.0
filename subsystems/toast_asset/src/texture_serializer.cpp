#include "toast_asset/texture_serializer.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::asset
{
	auto serializeTextureToFile(gpu::VKLogicalDevice &p_device, gpu::RawImage &p_image, const io::filesystem::Path &p_dst_path) -> void
	{
		const auto &image_spec_info{p_image.getSpecInfo()};

		TextureHeader out_header{};
		out_header.width        = image_spec_info.size.x;
		out_header.height       = image_spec_info.size.y;
		out_header.mipLevels    = image_spec_info.mipCount;
		out_header.vulkanFormat = static_cast<uint32>(image_spec_info.format);

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		const uint64 buffer_size{util::getBytesPerPixel(m_specInfo.format) * m_specInfo.size.x * m_specInfo.size.y};
		m_device->createBuffer(buffer_size, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							   staging_buffer, staging_buffer_memory);

		vk::ImageLayout previous_layout{m_currentImageLayout};
		util::transitionImageLayout(this, m_currentImageLayout, vk::ImageLayout::eTransferSrcOptimal);
		m_device->copyImageToBuffer(m_image, staging_buffer, {m_specInfo.size.x, m_specInfo.size.y, 1u}, m_specInfo.layerCount);
		util::transitionImageLayout(this, vk::ImageLayout::eTransferSrcOptimal, previous_layout);

		void *image_data{staging_buffer_memory.mapMemory(0u, buffer_size)};

		p_device.copyImageToBuffer(p_image.getImage(), *image_data, {image_spec_info.size.x, image_spec_info.size.y, 1u}, image_spec_info.layerCount);
	}
}
