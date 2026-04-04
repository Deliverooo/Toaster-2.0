#include "vk_image.hpp"

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKImage2D::VKImage2D(VKGPUContext *p_ctx, const ImageCreateInfo &p_create_info) : m_ctx(p_ctx), m_createInfo(p_create_info)
	{
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

	const ImageCreateInfo &VKImage2D::getCreateInfo() const
	{
		return m_createInfo;
	}

	void VKImage2D::resize(uint32 p_width, uint32 p_height)
	{
		m_createInfo.width  = p_width;
		m_createInfo.height = p_height;

		recreate();
	}

	void VKImage2D::recreate()
	{
		m_image       = nullptr;
		m_imageMemory = nullptr;
		m_imageView   = nullptr;
		m_sampler     = nullptr;

		const vk::Format format = getVulkanFormat(m_createInfo.format);

		vk::ImageUsageFlags usage_flags{vk::ImageUsageFlagBits::eSampled};
		if (m_createInfo.usage == EImageUsage::eAttachment)
		{
			if (m_ctx->isDepthFormat(format))
				usage_flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
			else
				usage_flags |= vk::ImageUsageFlagBits::eColorAttachment;
		}
		if (m_createInfo.usage == EImageUsage::eTexture)
			usage_flags |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
		else if (m_createInfo.usage == EImageUsage::eStorage)
			usage_flags |= vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst;

		vk::ImageAspectFlags aspect_mask{m_ctx->isDepthFormat(format) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor};
		if (m_ctx->hasStencilComponent(format))
			aspect_mask |= vk::ImageAspectFlagBits::eStencil;

		vk::ImageTiling         tiling{m_createInfo.usage == EImageUsage::eHostRead ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal};
		vk::MemoryPropertyFlags memory_property_flags{
			m_createInfo.usage == EImageUsage::eHostRead
				? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
				: vk::MemoryPropertyFlagBits::eDeviceLocal
		};

		m_ctx->createImage(m_createInfo.width, m_createInfo.height, format, tiling, usage_flags, memory_property_flags, m_image, m_imageMemory);

		m_imageView = m_ctx->createImageView(m_image, format, aspect_mask);

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.maxAnisotropy = 1.0f;

		sampler_create_info.minFilter  = vk::Filter::eLinear;
		sampler_create_info.magFilter  = vk::Filter::eLinear;
		sampler_create_info.mipmapMode = vk::SamplerMipmapMode::eLinear;

		sampler_create_info.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		sampler_create_info.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		sampler_create_info.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		sampler_create_info.mipLodBias   = 0.0f;
		sampler_create_info.minLod       = 0.0f;
		sampler_create_info.maxLod       = 100.0f;
		sampler_create_info.borderColor  = vk::BorderColor::eFloatCustomEXT;

		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};

		sampler_create_info.pNext = &border_colour_create_info;

		m_sampler = {m_ctx->getDevice(), sampler_create_info};
		_updateDescriptorInfo();
	}

	void VKImage2D::_updateDescriptorInfo()
	{
		const vk::Format format = getVulkanFormat(m_createInfo.format);

		if (m_ctx->isDepthFormat(format))
			m_descriptorImageInfo.imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
		else
			m_descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		if (m_createInfo.usage == EImageUsage::eStorage)
			m_descriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;
		else if (m_createInfo.usage == EImageUsage::eHostRead)
			m_descriptorImageInfo.imageLayout = vk::ImageLayout::eTransferDstOptimal;

		m_descriptorImageInfo.imageView = m_imageView;
		m_descriptorImageInfo.sampler   = m_sampler;
	}
}
