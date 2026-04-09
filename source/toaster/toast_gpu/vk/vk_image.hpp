#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "../image.hpp"
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	struct ImageCreateInfo
	{
		uint32                  width{0u};
		uint32                  height{0u};
		vk::Format              format{vk::Format::eUndefined};
		vk::ImageUsageFlags     usage{vk::ImageUsageFlagBits::eSampled};
		vk::SampleCountFlagBits sampleCount{vk::SampleCountFlagBits::e1};
		uint32                  mipCount{1u};
	};

	// An "Image" represents a texture without a sampler
	// You would use this for render attachments
	class VKImage2D
	{
	public:
		VKImage2D(VKGPUContext *p_ctx, const ImageCreateInfo &p_create_info);

		vk::raii::Image &       getImage();
		vk::raii::DeviceMemory &getImageMemory();
		vk::raii::ImageView &   getImageView();

		[[nodiscard]] const ImageCreateInfo &getCreateInfo() const;

		void resize(uint32 p_width, uint32 p_height);
		void recreate();

	private:
		VKGPUContext *m_ctx{nullptr};

		ImageCreateInfo m_createInfo{};

		vk::raii::Image        m_image{nullptr};
		vk::raii::DeviceMemory m_imageMemory{nullptr};
		vk::raii::ImageView    m_imageView{nullptr};
	};
}
