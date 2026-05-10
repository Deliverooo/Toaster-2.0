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

		auto resize(uint32 p_width, uint32 p_height) -> void;
		auto setData(void *p_data, uint64 p_size) -> void;
		auto setData(const Buffer &p_buffer) -> void;

		// If you want to work with textures and don't want to immediately set the data you might want to deffer the sampler creation
		auto createSampler(vk::ImageLayout p_override_layout = vk::ImageLayout::eUndefined) -> void; // Also creates the descriptor info

		[[nodiscard]] auto getSpecInfo() const -> const TextureSpecInfo &;
		auto               getPath() const -> const io::filesystem::Path &;
		[[nodiscard]] auto getMipLevelCount() const -> uint32;
		auto               getImage() -> RefPtr<VKRawImage>;
		[[nodiscard]] auto getSampler() -> vk::raii::Sampler &;
		[[nodiscard]] auto getDescriptorInfo() -> vk::DescriptorImageInfo &;

	private:
		TextureSpecInfo      m_specInfo{};
		io::filesystem::Path m_path;

		uint32 m_mipLevels{1u};

		RefPtr<VKRawImage> m_image{nullptr};
		vk::raii::Sampler  m_sampler{nullptr};

		Buffer m_textureData;

		vk::DescriptorImageInfo m_descriptorImageInfo{nullptr};
	};

	class TST_GPU_API VKTexture3D final : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(Texture3D)
	public:
		VKTexture3D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path);
		VKTexture3D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, Buffer p_data);

		auto resize(uint32 p_width, uint32 p_height) -> void;
		auto setData(void *p_data, uint64 p_size) -> void;
		auto setData(const Buffer &p_buffer) -> void;

		// If you want to work with textures and don't want to immediately set the data you might want to deffer the sampler creation
		auto createSampler(vk::ImageLayout p_override_layout = vk::ImageLayout::eUndefined) -> void; // Also creates the descriptor info

		auto               getPath() const -> const io::filesystem::Path &;
		[[nodiscard]] auto getSpecInfo() const -> const TextureSpecInfo &;
		auto               getImage() -> RefPtr<VKRawImage>;
		[[nodiscard]] auto getSampler() -> vk::raii::Sampler &;
		[[nodiscard]] auto getDescriptorInfo() -> vk::DescriptorImageInfo &;

	private:
		TextureSpecInfo m_specInfo{};

		io::filesystem::Path m_path;

		Buffer m_textureData;

		RefPtr<VKRawImage> m_image{nullptr};
		vk::raii::Sampler  m_sampler{nullptr};

		vk::DescriptorImageInfo m_descriptorImageInfo{nullptr};
	};
}
