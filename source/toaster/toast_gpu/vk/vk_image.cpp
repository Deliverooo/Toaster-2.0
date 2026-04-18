#include "vk_image.hpp"

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKImage2D::VKImage2D(VKGPUContext *p_ctx, const ImageCreateInfo &p_create_info) : m_ctx(p_ctx), m_createInfo(p_create_info)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");

		recreate();
	}

	auto VKImage2D::getContext() const -> VKGPUContext *
	{
		return m_ctx;
	}

	auto VKImage2D::getImage() -> vk::raii::Image &
	{
		return m_image;
	}

	auto VKImage2D::getImageMemory() -> vk::raii::DeviceMemory &
	{
		return m_imageMemory;
	}

	auto VKImage2D::getImageView() -> vk::raii::ImageView &
	{
		return m_imageView;
	}

	auto VKImage2D::getCreateInfo() const -> const ImageCreateInfo &
	{
		return m_createInfo;
	}

	auto VKImage2D::setCurrentImageLayout(vk::ImageLayout p_layout) -> void
	{
		m_currentImageLayout = p_layout;
	}

	auto VKImage2D::getCurrentImageLayout() const -> vk::ImageLayout
	{
		return m_currentImageLayout;
	}

	#if 0
	auto VKImage2D::setData(void *p_data, uint64 p_size) -> void
	{
		TST_ASSERT_MSG(p_data, "p_data is nullptr");

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		vk::DeviceSize image_size{p_size}; // 1 Pixel * 1 Pixel * RGBA

		m_ctx->createBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							staging_buffer, staging_buffer_memory);

		void *mapped = staging_buffer_memory.mapMemory(0, image_size, {});
		std::memcpy(mapped, p_data, image_size);
		staging_buffer_memory.unmapMemory();

		vk::Format image_format = vk::Format::eR8G8B8A8Unorm;

		m_ctx->createImage(m_createInfo.width, m_createInfo.height, m_createInfo.mipCount, vk::SampleCountFlagBits::e1, image_format, vk::ImageTiling::eOptimal,
						   vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
						   vk::MemoryPropertyFlagBits::eDeviceLocal, m_image, m_imageMemory);

		m_ctx->transitionImageLayout(m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits::eNone,
									 vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
									 m_createInfo.mipCount, vk::ImageAspectFlagBits::eColor);

		m_ctx->copyBufferToImage(staging_buffer, m_image, m_createInfo.width, m_createInfo.height);

		m_ctx->transitionImageLayout(m_image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eTransferWrite,
									 vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
									 m_createInfo.mipCount, vk::ImageAspectFlagBits::eColor);

		m_imageView = m_ctx->createImageView(m_image, image_format, vk::ImageAspectFlagBits::eColor, m_createInfo.mipCount);

		auto physical_device_props = m_ctx->getPhysicalDevice().getProperties();

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
		sampler_create_info.borderColor             = vk::BorderColor::eIntOpaqueBlack;
		sampler_create_info.unnormalizedCoordinates = false;
		m_sampler                                   = {m_ctx->getDevice(), sampler_create_info};

		_updateDescriptorInfo();
	}
	#endif

	auto VKImage2D::resize(uint32 p_width, uint32 p_height) -> void
	{
		m_createInfo.width  = p_width;
		m_createInfo.height = p_height;

		recreate();
	}

	auto VKImage2D::recreate() -> void
	{
		m_image       = nullptr;
		m_imageMemory = nullptr;
		m_imageView   = nullptr;

		m_currentImageLayout = vk::ImageLayout::eUndefined;

		m_ctx->createImage(m_createInfo.width, m_createInfo.height, m_createInfo.mipCount, m_createInfo.sampleCount, m_createInfo.format, vk::ImageTiling::eOptimal,
						   m_createInfo.usage, vk::MemoryPropertyFlagBits::eDeviceLocal, m_image, m_imageMemory);

		vk::ImageAspectFlags aspect_flags{m_ctx->isDepthFormat(m_createInfo.format) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor};
		if (m_ctx->hasStencilComponent(m_createInfo.format))
			aspect_flags |= vk::ImageAspectFlagBits::eStencil;

		if (m_createInfo.usage & vk::ImageUsageFlagBits::eColorAttachment)
		{
			m_ctx->transitionImageLayout(m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits2::eNone,
										 vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eNone,
										 vk::PipelineStageFlagBits2::eColorAttachmentOutput, m_createInfo.mipCount, aspect_flags);
			m_currentImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		}
		else if (m_createInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
		{
			m_ctx->transitionImageLayout(m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits2::eNone,
										 vk::AccessFlagBits2::eDepthStencilAttachmentWrite, vk::PipelineStageFlagBits2::eNone,
										 vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, m_createInfo.mipCount,
										 aspect_flags);
			m_currentImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		}

		m_imageView = m_ctx->createImageView(m_image, m_createInfo.format, aspect_flags, m_createInfo.mipCount);
	}
}
