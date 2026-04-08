#include "vk_image.hpp"

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKImage2D::VKImage2D(VKGPUContext *p_ctx, const ImageCreateInfo &p_create_info) : m_ctx(p_ctx), m_createInfo(p_create_info)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");
	}

	vk::raii::Image &VKImage2D::getImage()
	{
		return m_image;
	}

	vk::raii::DeviceMemory &VKImage2D::getImageMemory()
	{
		return m_imageMemory;
	}

	vk::raii::ImageView &VKImage2D::getImageView()
	{
		return m_imageView;
	}

	vk::raii::Sampler &VKImage2D::getSampler()
	{
		return m_sampler;
	}

	vk::DescriptorImageInfo &VKImage2D::getDescriptorInfo()
	{
		return m_descriptorImageInfo;
	}

	const ImageCreateInfo &VKImage2D::getCreateInfo() const
	{
		return m_createInfo;
	}

	void VKImage2D::setData(void *p_data, uint64 p_size)
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

		m_ctx->createImage(m_createInfo.width, m_createInfo.height, m_createInfo.mips, vk::SampleCountFlagBits::e1, image_format, vk::ImageTiling::eOptimal,
						   vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
						   vk::MemoryPropertyFlagBits::eDeviceLocal, m_image, m_imageMemory);

		m_ctx->transitionImageLayout(m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits::eNone,
									 vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, m_createInfo.mips,
									 vk::ImageAspectFlagBits::eColor);

		m_ctx->copyBufferToImage(staging_buffer, m_image, m_createInfo.width, m_createInfo.height);

		m_ctx->transitionImageLayout(m_image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eTransferWrite,
									 vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, m_createInfo.mips,
									 vk::ImageAspectFlagBits::eColor);

		m_imageView = m_ctx->createImageView(m_image, image_format, vk::ImageAspectFlagBits::eColor, m_createInfo.mips);

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

	void VKImage2D::resize(uint32 p_width, uint32 p_height)
	{
		m_createInfo.width  = p_width;
		m_createInfo.height = p_height;

		recreate();
	}

	void VKImage2D::recreate()
	{
	}

	void VKImage2D::_updateDescriptorInfo()
	{
		m_descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_descriptorImageInfo.imageView   = m_imageView;
		m_descriptorImageInfo.sampler     = m_sampler;
	}
}
