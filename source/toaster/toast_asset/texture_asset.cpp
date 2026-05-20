#include "texture_asset.hpp"

namespace toaster::asset
{
	Texture2DAsset::Texture2DAsset(const gpu::Texture2DHandle &p_texture) : m_texture(p_texture)
	{
	}

	auto Texture2DAsset::getTexture() const -> const gpu::Texture2DHandle &
	{
		return m_texture;
	}

	Texture3DAsset::Texture3DAsset(const gpu::Texture3DHandle &p_texture) : m_texture(p_texture)
	{
	}

	auto Texture3DAsset::getTexture() const -> const gpu::Texture3DHandle &
	{
		return m_texture;
	}
}
