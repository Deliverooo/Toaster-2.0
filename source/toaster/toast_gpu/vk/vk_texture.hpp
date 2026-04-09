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
		uint32     width{0u};
		uint32     height{0u};
		vk::Format format{vk::Format::eUndefined};
		bool       generateMips{true};
	};

	class VKTexture2D final : public IGPUResource
	{
	public:
		VKTexture2D(VKGPUContext *p_ctx, const TextureSpecInfo &p_spec_info);
		VKTexture2D(VKGPUContext *p_ctx, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path);
		VKTexture2D(VKGPUContext *p_ctx, const TextureSpecInfo &p_spec_info, void *p_data, uint64 p_size);

		void resize(uint32 p_width, uint32 p_height);

		[[nodiscard]] const TextureSpecInfo &  getSpecInfo() const;
		const io::filesystem::Path &           getPath() const;
		[[nodiscard]] uint32                   getMipLevelCount() const;
		const RefPtr<VKImage2D> &              getImage() const;
		[[nodiscard]] vk::raii::Sampler &      getSampler();
		[[nodiscard]] vk::DescriptorImageInfo &getDescriptorInfo();
		vk::ImageLayout                        getCurrentImageLayout() const;

		[[nodiscard]] EGPUResourceType getResourceType() const override;

	private:
		VKGPUContext *m_ctx{nullptr};

		TextureSpecInfo      m_specInfo{};
		io::filesystem::Path m_path;

		uint32 m_mipLevels{1u};

		RefPtr<VKImage2D> m_image{nullptr};
		vk::raii::Sampler m_sampler{nullptr};

		vk::DescriptorImageInfo m_descriptorImageInfo{nullptr};

		vk::ImageLayout m_currentImageLayout{vk::ImageLayout::eUndefined};
	};
}
