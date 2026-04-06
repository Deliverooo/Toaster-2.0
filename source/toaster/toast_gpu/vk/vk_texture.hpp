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
		uint32       width{0u};
		uint32       height{0u};
		EImageFormat format{EImageFormat::eRGBA};
	};

	class VKTexture2D final : public IResource
	{
	public:
		VKTexture2D(VKGPUContext *p_ctx, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path);

		vk::raii::Image &       getImage();
		vk::raii::DeviceMemory &getImageMemory();
		vk::raii::ImageView &   getImageView();
		vk::raii::Sampler &     getSampler();

		vk::DescriptorImageInfo& getDescriptorInfo();

		[[nodiscard]] const TextureSpecInfo &getSpecInfo() const;

		EResourceType getResourceType() const override;

	private:
		VKGPUContext *m_ctx{nullptr};

		TextureSpecInfo m_specInfo{};

		vk::raii::Image        m_image{nullptr};
		vk::raii::DeviceMemory m_imageMemory{nullptr};
		vk::raii::ImageView    m_imageView{nullptr};
		vk::raii::Sampler      m_sampler{nullptr};

		vk::DescriptorImageInfo m_descriptorImageInfo{nullptr};
	};
}
