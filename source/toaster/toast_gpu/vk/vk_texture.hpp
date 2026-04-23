#pragma once

#include "vk_image.hpp"
#include "toast_gpu/resource.hpp"
#include "toast_lib/buffer.hpp"
#include "toast_lib/ptr.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	struct TextureSpecInfo
	{
		uint32                  width{0u};
		uint32                  height{0u};
		vk::Format              format{vk::Format::eR8G8B8A8Srgb};
		vk::SampleCountFlagBits sampleCount{vk::SampleCountFlagBits::e1};

		bool generateMips{true};
	};

	class TST_GPU_API VKTexture2D final : public IGPUResource
	{
	public:
		VKTexture2D(VKLogicalDevice *p_dev, const TextureSpecInfo &p_spec_info);
		VKTexture2D(VKLogicalDevice *p_dev, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path);
		VKTexture2D(VKLogicalDevice *p_dev, const TextureSpecInfo &p_spec_info, void *p_data, uint64 p_size);
		auto getDevice() const -> VKLogicalDevice *;

		auto resize(uint32 p_width, uint32 p_height) -> void;

		[[nodiscard]] auto getSpecInfo() const -> const TextureSpecInfo &;
		auto               getPath() const -> const io::filesystem::Path &;
		[[nodiscard]] auto getMipLevelCount() const -> uint32;
		auto               getImage() const -> const RefPtr<VKImage2D> &;
		[[nodiscard]] auto getSampler() -> vk::raii::Sampler &;
		[[nodiscard]] auto getDescriptorInfo() -> vk::DescriptorImageInfo &;

		[[nodiscard]] auto getResourceType() const -> EGPUResourceType override;

	private:
		VKLogicalDevice *m_device{nullptr};

		TextureSpecInfo      m_specInfo{};
		io::filesystem::Path m_path;

		uint32 m_mipLevels{1u};

		RefPtr<VKImage2D> m_image{nullptr};
		vk::raii::Sampler m_sampler{nullptr};

		vk::DescriptorImageInfo m_descriptorImageInfo{nullptr};
	};

	// TODO: Actually implement ts... (I am very scared of equirectangular to cubemap conversions...)
	class TST_GPU_API VKTexture3D final : public IGPUResource
	{
	public:
		VKTexture3D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, void *p_data, uint64 p_size);
		[[nodiscard]] auto getDevice() const -> VKLogicalDevice *;

		[[nodiscard]] auto getSpecInfo() const -> const TextureSpecInfo &;
		[[nodiscard]] auto getImage() -> vk::raii::Image &;
		[[nodiscard]] auto getImageView() -> vk::raii::ImageView &;
		[[nodiscard]] auto getDescriptorInfo() -> vk::DescriptorImageInfo &;

		[[nodiscard]] auto getResourceType() const -> EGPUResourceType override;

	private:
		VKLogicalDevice *m_device{nullptr};

		TextureSpecInfo m_specInfo{};

		vk::raii::Image        m_image{nullptr};
		vk::raii::DeviceMemory m_imageMemory{nullptr};
		vk::raii::ImageView    m_imageView{nullptr};

		vk::DescriptorImageInfo m_descriptorImageInfo{nullptr};
	};
}
