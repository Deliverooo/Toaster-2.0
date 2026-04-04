#pragma once

#include "vk_image.hpp"
#include "toast_lib/buffer.hpp"
#include "toast_lib/ptr.hpp"

namespace toaster::gpu
{
	struct TextureCreateInfo
	{
		uint32       width{0u};
		uint32       height{0u};
		EImageFormat format{EImageFormat::eRGBA};
	};

	class VKTexture2D
	{
	public:
		VKTexture2D(VKGPUContext *p_ctx, const TextureCreateInfo &p_create_info);

		RefPtr<VKImage2D> getImage() const;

		const TextureCreateInfo &getCreateInfo() const;

		Buffer getImageData();

	private:
		VKGPUContext *m_ctx{nullptr};

		TextureCreateInfo m_createInfo{};

		Buffer            m_imageData{};
		RefPtr<VKImage2D> m_image;
	};
}
