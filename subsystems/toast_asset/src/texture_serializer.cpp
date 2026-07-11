#include "toast_asset/texture_serializer.hpp"

#include <fstream>

#include "../../toast_render/include/toast_render/render_context.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_lib/io/file_stream.hpp"

namespace toaster::asset
{
	TextureSerializer::TextureSerializer(render::RenderContext &p_render_ctx) : m_renderCtx(&p_render_ctx)
	{
	}

	auto TextureSerializer::serializeTextureToFile(gpu::RawImage &p_image, const io::filesystem::Path &p_dst_path) -> void
	{
		const auto &image_spec_info{p_image.getSpecInfo()};

		TextureHeader out_header{};
		out_header.width        = image_spec_info.size.x;
		out_header.height       = image_spec_info.size.y;
		out_header.mipLevels    = image_spec_info.mipCount;
		out_header.vulkanFormat = static_cast<uint32>(image_spec_info.format);

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		const uint64 buffer_size{gpu::util::getBytesPerPixel(image_spec_info.format) * image_spec_info.size.x * image_spec_info.size.y};
		out_header.dataSize = buffer_size;

		m_renderCtx->getLogicalDevice()->createBuffer(staging_buffer, staging_buffer_memory, buffer_size, vk::BufferUsageFlagBits2::eTransferDst,
													  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		vk::ImageLayout previous_layout{p_image.getCurrentImageLayout()};
		gpu::util::transitionImageLayout(&p_image, previous_layout, vk::ImageLayout::eTransferSrcOptimal);
		m_renderCtx->getGPUContext()->copyImageToBuffer(p_image.getImage(), staging_buffer, {image_spec_info.size.x, image_spec_info.size.y, 1u},
														   image_spec_info.layerCount);
		gpu::util::transitionImageLayout(&p_image, vk::ImageLayout::eTransferSrcOptimal, previous_layout);

		void *image_data{staging_buffer_memory.mapMemory(0u, buffer_size)};

		io::FileStreamWriter out{p_dst_path};
		out.writeRaw(out_header);
		out.setStreamPos(sizeof(TextureHeader));
		out.writeData(static_cast<const uint8 *>(image_data), buffer_size);

		staging_buffer_memory.unmapMemory();
	}
}
