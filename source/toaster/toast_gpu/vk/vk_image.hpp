#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "../image.hpp"
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	struct ImageCreateInfo
	{
		vk::Format format{vk::Format::eUndefined};
		uint32     width{0u};
		uint32     height{0u};
		uint32     mips{1u};
	};

	class VKImage2D
	{
	public:
		VKImage2D(VKGPUContext *p_ctx, const ImageCreateInfo &p_create_info);

		vk::raii::Image &       getImage();
		vk::raii::DeviceMemory &getImageMemory();
		vk::raii::ImageView &   getImageView();
		vk::raii::Sampler &     getSampler();

		vk::DescriptorImageInfo &getDescriptorInfo();

		[[nodiscard]] const ImageCreateInfo &getCreateInfo() const;

		void setData(void *p_data, uint64 p_size);
		void resize(uint32 p_width, uint32 p_height);
		void recreate();

	private:
		void _updateDescriptorInfo();

		VKGPUContext *m_ctx{nullptr};

		ImageCreateInfo m_createInfo{};

		vk::raii::Image        m_image{nullptr};
		vk::raii::DeviceMemory m_imageMemory{nullptr};
		vk::raii::ImageView    m_imageView{nullptr};
		vk::raii::Sampler      m_sampler{nullptr};

		vk::DescriptorImageInfo m_descriptorImageInfo{};
	};

	inline vk::Format getVulkanFormat(EImageFormat p_format)
	{
		switch (p_format)
		{
			case EImageFormat::eR8UNorm: return vk::Format::eR8Unorm;
			case EImageFormat::eR8UInt: return vk::Format::eR8Uint;
			case EImageFormat::eR16UInt: return vk::Format::eR16Uint;
			case EImageFormat::eR32UInt: return vk::Format::eR32Uint;
			case EImageFormat::eR32F: return vk::Format::eR32Sfloat;
			case EImageFormat::eRG8: return vk::Format::eR8G8Unorm;
			case EImageFormat::eRG16F: return vk::Format::eR16G16Sfloat;
			case EImageFormat::eRG32F: return vk::Format::eR32G32Sfloat;
			case EImageFormat::eRGB: return vk::Format::eR8G8B8Unorm;
			case EImageFormat::eRGBA: return vk::Format::eR8G8B8A8Srgb;
			case EImageFormat::eRGBA16F: return vk::Format::eR16G16B16A16Sfloat;
			case EImageFormat::eRGBA32F: return vk::Format::eR32G32B32A32Sfloat;
			case EImageFormat::eB10R11G11UF: return vk::Format::eB10G11R11UfloatPack32;
			case EImageFormat::eSRGB: return vk::Format::eR8G8B8Srgb;
			case EImageFormat::eSRGBA: return vk::Format::eR8G8B8A8Srgb;
			case EImageFormat::eDepth32FStencil8UInt: return vk::Format::eD32SfloatS8Uint;
			case EImageFormat::eDepth32F: return vk::Format::eD32Sfloat;
			case EImageFormat::eDepth24Stencil8: return vk::Format::eD24UnormS8Uint;
		}
		return vk::Format::eUndefined;
	}
}
