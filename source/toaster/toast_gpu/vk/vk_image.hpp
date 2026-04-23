#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "../image.hpp"
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;

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
		VKImage2D(VKLogicalDevice *p_ctx, const ImageCreateInfo &p_create_info);
		[[nodiscard]] auto getDevice() const -> VKLogicalDevice *;

		auto getImage() -> vk::raii::Image &;
		auto getImageMemory() -> vk::raii::DeviceMemory &;
		auto getImageView() -> vk::raii::ImageView &;

		[[nodiscard]] auto getCreateInfo() const -> const ImageCreateInfo &;

		auto setCurrentImageLayout(vk::ImageLayout p_layout) -> void;
		auto getCurrentImageLayout() const -> vk::ImageLayout;

		auto resize(uint32 p_width, uint32 p_height) -> void;
		auto recreate() -> void;

	private:
		VKLogicalDevice *m_device{nullptr};

		ImageCreateInfo m_createInfo{};

		vk::raii::Image        m_image{nullptr};
		vk::raii::DeviceMemory m_imageMemory{nullptr};
		vk::raii::ImageView    m_imageView{nullptr};

		vk::ImageLayout m_currentImageLayout{vk::ImageLayout::eUndefined};
	};
}
