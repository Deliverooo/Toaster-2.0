#pragma once

#include "../toast_gpu.hpp"

#include <vulkan/vulkan_raii.hpp>

#include "toast_gpu/resource.hpp"
#include "toast_lib/buffer.hpp"
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

	class TST_GPU_API VKImage2D : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(Image2D)
	public:
		VKImage2D(VKLogicalDevice *p_device, const ImageSpecInfo &p_spec_info);

		auto resize(uint32 p_width, uint32 p_height) -> void;
		auto setData(void *p_data, uint64 p_size) -> void;

		// If you want to work with textures and don't want to immediately set the data you might want to deffer the sampler creation
		auto createSampler(vk::ImageLayout p_override_layout = vk::ImageLayout::eUndefined) -> void; // Also creates the descriptor info

		auto getImage() const -> const RefPtr<VKRawImage> &;
		auto getDescriptorInfo() const -> const vk::DescriptorImageInfo &;

	private:
		RefPtr<VKRawImage> m_image{nullptr};

		vk::raii::Sampler m_sampler{nullptr};
		Buffer            m_imageData;

		vk::DescriptorImageInfo m_descriptorImageInfo{};
	};

	// For dynamic rendering I have to handle the image layout transitions manually, so this simplifies things...
	namespace util
	{
		TST_GPU_API auto shaderReadToColourAttachment(AttachmentImage *p_image) -> void;
		TST_GPU_API auto shaderReadToDepthAttachment(AttachmentImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto shaderReadToTransferSrc(AttachmentImage *p_image) -> void;
		TST_GPU_API auto shaderReadToTransferDst(AttachmentImage *p_image) -> void;

		TST_GPU_API auto colourAttachmentToShaderRead(AttachmentImage *p_image) -> void;
		TST_GPU_API auto depthAttachmentToShaderRead(AttachmentImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto transferSrcToShaderRead(AttachmentImage *p_image) -> void;
		TST_GPU_API auto transferDstToShaderRead(AttachmentImage *p_image) -> void;

		TST_GPU_API auto undefinedToColourAttachment(AttachmentImage *p_image) -> void;
		TST_GPU_API auto undefinedToDepthAttachment(AttachmentImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto undefinedToTransferSrc(AttachmentImage *p_image) -> void;
		TST_GPU_API auto undefinedToTransferDst(AttachmentImage *p_image) -> void;
		TST_GPU_API auto undefinedToGeneral(AttachmentImage *p_image) -> void;
	}
}
