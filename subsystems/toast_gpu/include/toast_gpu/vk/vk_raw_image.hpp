#pragma once

#include "toast_gpu/toast_gpu.hpp"

#include <vulkan/vulkan_raii.hpp>

#include "toast_lib/buffer.hpp"
#include "toast_lib/core_basic.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;

	struct ImageSpecInfo
	{
		tsm::uint2              size{0u};
		vk::Format              format{vk::Format::eUndefined};
		vk::ImageUsageFlags     usage{vk::ImageUsageFlagBits::eSampled};
		vk::SampleCountFlagBits sampleCount{vk::SampleCountFlagBits::e1};
		uint32                  mipCount{1u};
		uint32                  layerCount{1u};
	};

	// An "Image" represents a texture without a sampler
	// You would use this for render attachments
	class TST_GPU_API VKRawImage
	{
		TST_GPU_OBJECT
	public:
		VKRawImage(VKLogicalDevice *p_ctx, const ImageSpecInfo &p_spec_info);
		~VKRawImage();

		auto getImage() -> vk::Image &;
		auto getImageMemory() -> vk::DeviceMemory &;
		auto getImageView() -> vk::ImageView &;

		[[nodiscard]] auto getSpecInfo() const -> const ImageSpecInfo &;

		[[nodiscard]] auto isMultisample() const -> bool;

		auto               setCurrentImageLayout(vk::ImageLayout p_layout) -> void;
		[[nodiscard]] auto getCurrentImageLayout() const -> vk::ImageLayout;

		auto saveToFile(const io::filesystem::Path &p_path) -> void;

		auto setData(void *p_data, uint64 p_size) -> void;
		auto setData(const Buffer &p_buffer) -> void;
		auto resize(tsm::uint2 p_size) -> void;

		auto recreate() -> void;

	private:
		ImageSpecInfo m_specInfo{};

		vk::Image        m_image{nullptr};
		vk::DeviceMemory m_imageMemory{nullptr};
		vk::ImageView    m_imageView{nullptr};

		vk::ImageLayout m_currentImageLayout{vk::ImageLayout::eUndefined};
	};

	TST_GPU_DEFINE_HANDLE(VKRawImage, RawImage)

	namespace util
	{
		TST_GPU_API auto loadTextureImage(const io::filesystem::Path &p_path, vk::Format &p_out_format, uint32 &p_out_width, uint32 &p_out_height) -> Buffer;

		// For dynamic rendering I have to handle the image layout transitions manually, so this simplifies things...
		// Prefer this over the other functions
		TST_GPU_API auto transitionImageLayout(VKRawImage *      p_image, vk::ImageLayout p_src_layout, vk::ImageLayout p_dst_layout,
											   vk::CommandBuffer p_override_command_buffer = nullptr) -> void;

		TST_GPU_API auto colourAttachmentToShaderRead(VKRawImage *p_image) -> void;
		TST_GPU_API auto colourAttachmentToTransferSrc(VKRawImage *p_image) -> void;
		TST_GPU_API auto colourAttachmentToTransferDst(VKRawImage *p_image) -> void;
		TST_GPU_API auto colourAttachmentToGeneral(VKRawImage *p_image) -> void;

		TST_GPU_API auto depthAttachmentToShaderRead(VKRawImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto depthAttachmentToTransferSrc(VKRawImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto depthAttachmentToTransferDst(VKRawImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto depthAttachmentToGeneral(VKRawImage *p_image, bool p_read_only) -> void;

		TST_GPU_API auto shaderReadToColourAttachment(VKRawImage *p_image) -> void;
		TST_GPU_API auto shaderReadToDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto shaderReadToTransferSrc(VKRawImage *p_image) -> void;
		TST_GPU_API auto shaderReadToTransferDst(VKRawImage *p_image) -> void;
		TST_GPU_API auto shaderReadToGeneral(VKRawImage *p_image) -> void;

		TST_GPU_API auto transferSrcToColourAttachment(VKRawImage *p_image) -> void;
		TST_GPU_API auto transferSrcToDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto transferSrcToShaderRead(VKRawImage *p_image) -> void;
		TST_GPU_API auto transferSrcToTransferDst(VKRawImage *p_image) -> void;
		TST_GPU_API auto transferSrcToGeneral(VKRawImage *p_image) -> void;

		TST_GPU_API auto transferDstToColourAttachment(VKRawImage *p_image) -> void;
		TST_GPU_API auto transferDstToDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto transferDstToShaderRead(VKRawImage *p_image) -> void;
		TST_GPU_API auto transferDstToTransferSrc(VKRawImage *p_image) -> void;
		TST_GPU_API auto transferDstToGeneral(VKRawImage *p_image) -> void;

		TST_GPU_API auto generalToColourAttachment(VKRawImage *p_image) -> void;
		TST_GPU_API auto generalToDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto generalToShaderRead(VKRawImage *p_image) -> void;
		TST_GPU_API auto generalToTransferSrc(VKRawImage *p_image) -> void;
		TST_GPU_API auto generalToTransferDst(VKRawImage *p_image) -> void;

		TST_GPU_API auto undefinedToColourAttachment(VKRawImage *p_image) -> void;
		TST_GPU_API auto undefinedToDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto undefinedToShaderRead(VKRawImage *p_image) -> void;
		TST_GPU_API auto undefinedToTransferSrc(VKRawImage *p_image) -> void;
		TST_GPU_API auto undefinedToTransferDst(VKRawImage *p_image) -> void;
		TST_GPU_API auto undefinedToGeneral(VKRawImage *p_image) -> void;

		TST_GPU_API auto toColourAttachment(VKRawImage *p_image) -> void;
		TST_GPU_API auto toDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void;
		TST_GPU_API auto toShaderRead(VKRawImage *p_image) -> void;
		TST_GPU_API auto toTransferSrc(VKRawImage *p_image) -> void;
		TST_GPU_API auto toTransferDst(VKRawImage *p_image) -> void;
		TST_GPU_API auto toGeneral(VKRawImage *p_image) -> void;
	}
}
