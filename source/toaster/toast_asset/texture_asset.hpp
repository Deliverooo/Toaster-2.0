#pragma once

#include "asset.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_render/render_context.hpp"

namespace toaster::asset
{
	class TST_API Texture2DAsset : public Asset
	{
		TST_ASSET(Texture2D)
	public:
		Texture2DAsset(const gpu::Texture2DHandle &p_texture);
		virtual ~Texture2DAsset() override = default;

		auto getTexture() const -> const gpu::Texture2DHandle &;

	private:
		gpu::Texture2DHandle m_texture{nullptr};
	};

	class TST_API Texture3DAsset : public Asset
	{
		TST_ASSET(Texture3D)
	public:
		Texture3DAsset(const gpu::Texture3DHandle &p_texture);
		virtual ~Texture3DAsset() override = default;

		auto getTexture() const -> const gpu::Texture3DHandle &;

	private:
		gpu::Texture3DHandle m_texture{nullptr};
	};
}
