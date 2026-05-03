#pragma once

#include "../toast_gpu.hpp"

#include <vulkan/vulkan_raii.hpp>

#include "toast_gpu/resource.hpp"
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;

	struct ImageSpecInfo
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
	class TST_GPU_API VKRawImage
	{
		TST_GPU_OBJECT
	public:
		VKRawImage(VKLogicalDevice *p_ctx, const ImageSpecInfo &p_spec_info);

		auto getImage() -> vk::raii::Image &;
		auto getImageMemory() -> vk::raii::DeviceMemory &;
		auto getImageView() -> vk::raii::ImageView &;

		[[nodiscard]] auto getSpecInfo() const -> const ImageSpecInfo &;

		auto setCurrentImageLayout(vk::ImageLayout p_layout) -> void;
		auto getCurrentImageLayout() const -> vk::ImageLayout;

		auto resize(uint32 p_width, uint32 p_height) -> void;
		auto recreate() -> void;

	private:
		ImageSpecInfo m_specInfo{};

		vk::raii::Image        m_image{nullptr};
		vk::raii::DeviceMemory m_imageMemory{nullptr};
		vk::raii::ImageView    m_imageView{nullptr};

		vk::ImageLayout m_currentImageLayout{vk::ImageLayout::eUndefined};
	};

	using AttachmentImage       = VKRawImage;
	using AttachmentImageHandle = RefPtr<VKRawImage>;

	// TODO: Probably finish ts... however I don#t know when I will need to use just an image in a shader...
	class TST_GPU_API VKImage2D : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(Image2D)
	public:
		VKImage2D(VKLogicalDevice *p_device, const RefPtr<VKRawImage> &p_image);

		auto getImage() const -> const RefPtr<VKRawImage> &;
		auto getDescriptorInfo() const -> const vk::DescriptorImageInfo &;

	private:
		RefPtr<VKRawImage> m_image{nullptr};

		vk::DescriptorImageInfo m_descriptorImageInfo{};
	};

	// For dynamic rendering I have to handle the image layout transitions manually, so this simplifies things...
	namespace util
	{
		auto shaderReadToColourAttachment(AttachmentImage *p_image) -> void;
		auto shaderReadToDepthAttachment(AttachmentImage *p_image, bool p_read_only) -> void;
		auto colourAttachmentToShaderRead(AttachmentImage *p_image) -> void;
		auto depthAttachmentToShaderRead(AttachmentImage *p_image, bool p_read_only) -> void;
		auto transferDstToShaderRead(AttachmentImage *p_image) -> void;
		auto shaderReadToTransferDst(AttachmentImage *p_image) -> void;
		auto undefinedToTransferDst(AttachmentImage *p_image) -> void;
	}
}
