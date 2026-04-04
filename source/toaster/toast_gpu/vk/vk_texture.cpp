#include "vk_texture.hpp"

namespace toaster::gpu
{
	VKTexture2D::VKTexture2D(VKGPUContext *p_ctx, const TextureCreateInfo &p_create_info) : m_ctx(p_ctx), m_createInfo(p_create_info)
	{
		ImageCreateInfo image_create_info{};
		image_create_info.width = m_createInfo.width;
		image_create_info.width = m_createInfo.height;
		image_create_info.format = m_createInfo.format;
		image_create_info.usage = EImageUsage::eTexture;
	}

	RefPtr<VKImage2D> VKTexture2D::getImage() const
	{
		return m_image;
	}

	const TextureCreateInfo &VKTexture2D::getCreateInfo() const
	{
		return m_createInfo;
	}

	Buffer VKTexture2D::getImageData()
	{
		return m_imageData;
	}
}
