#pragma once

#include "vk_image.hpp"
#include "toast_gpu/resource.hpp"
#include "toast_lib/buffer.hpp"
#include "toast_lib/ptr.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	enum class ETextureUsage
	{
		eRenderAttachmentSampled, eShaderSampled
	};

	struct TextureSpecInfo
	{
		uint32                  width{0u};
		uint32                  height{0u};
		vk::Format              format{vk::Format::eR8G8B8A8Srgb};
		vk::SampleCountFlagBits sampleCount{vk::SampleCountFlagBits::e1};
		ETextureUsage           usage{ETextureUsage::eRenderAttachmentSampled};

		bool generateMips{true};
	};

	class TST_GPU_API VKTexture2D final : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(Texture2D)
	public:
		VKTexture2D(VKLogicalDevice *p_dev, const TextureSpecInfo &p_spec_info);
		VKTexture2D(VKLogicalDevice *p_dev, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path);
		VKTexture2D(VKLogicalDevice *p_dev, const TextureSpecInfo &p_spec_info, void *p_data, uint64 p_size);
		virtual ~VKTexture2D() override;

		auto resize(uint32 p_width, uint32 p_height) -> void;
		auto setData(void *p_data, uint64 p_size) -> void;
		auto setData(const Buffer &p_buffer) -> void;

		// If you want to work with textures and don't want to immediately set the data you might want to deffer the sampler creation
		auto createSampler(vk::ImageLayout p_override_layout = vk::ImageLayout::eUndefined) -> void; // Also creates the descriptor info

		[[nodiscard]] auto getSpecInfo() const -> const TextureSpecInfo &;
		auto               getPath() const -> const io::filesystem::Path &;
		[[nodiscard]] auto getMipLevelCount() const -> uint32;
		auto               getImage() -> RefPtr<VKRawImage>;
		[[nodiscard]] auto getSampler() -> vk::Sampler &;
		[[nodiscard]] auto getDescriptorInfo() -> vk::DescriptorImageInfo &;
		[[nodiscard]] auto getDescriptorInfo() const -> const vk::DescriptorImageInfo &;

	private:
		TextureSpecInfo      m_specInfo{};
		io::filesystem::Path m_path;

		uint32 m_mipLevels{1u};

		RefPtr<VKRawImage> m_image{nullptr};
		vk::Sampler     m_sampler{nullptr};

		Buffer m_textureData;

		vk::DescriptorImageInfo m_descriptorImageInfo{nullptr};
	};

	TST_GPU_DEFINE_HANDLE(VKTexture2D, Texture2D);

	class TST_GPU_API VKTexture3D final : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(Texture3D)
	public:
		VKTexture3D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info);
		VKTexture3D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path);
		VKTexture3D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, const Buffer &p_data);
		virtual ~VKTexture3D() override;

		auto resize(uint32 p_width, uint32 p_height) -> void;
		auto setData(void *p_data, uint64 p_size) -> void;
		auto setData(const Buffer &p_buffer) -> void;

		// If you want to work with textures and don't want to immediately set the data you might want to deffer the sampler creation
		auto createSampler(vk::ImageLayout p_override_layout = vk::ImageLayout::eUndefined) -> void; // Also creates the descriptor info

		auto               getPath() const -> const io::filesystem::Path &;
		[[nodiscard]] auto getSpecInfo() const -> const TextureSpecInfo &;
		auto               getImage() -> RefPtr<VKRawImage>;
		[[nodiscard]] auto getSampler() -> vk::Sampler &;
		[[nodiscard]] auto getDescriptorInfo() -> vk::DescriptorImageInfo &;
		[[nodiscard]] auto getDescriptorInfo() const -> const vk::DescriptorImageInfo &;

	private:
		TextureSpecInfo m_specInfo{};

		io::filesystem::Path m_path;

		Buffer m_textureData;

		RefPtr<VKRawImage> m_image{nullptr};
		vk::Sampler  m_sampler{nullptr};

		vk::DescriptorImageInfo m_descriptorImageInfo{nullptr};
	};

	TST_GPU_DEFINE_HANDLE(VKTexture3D, Texture3D);

	namespace util
	{
		TST_GPU_API auto loadTextureImage(const io::filesystem::Path &p_path, vk::Format &p_out_format, uint32 &p_out_width, uint32 &p_out_height) -> Buffer;
	}
}
