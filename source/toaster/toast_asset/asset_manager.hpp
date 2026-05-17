#pragma once

#include "asset.hpp"
#include "toast_lib/ptr.hpp"

namespace toaster::asset
{
	class AssetManager
	{
	public:
		AssetManager();

		[[nodiscard]] auto getAsset(AssetID p_asset_id) const -> RefPtr<Asset>;

		template<typename TAsset>
		[[nodiscard]] auto getAsset(AssetID p_asset_id) const -> RefPtr<TAsset>
		{
			return getAsset(p_asset_id).as<TAsset>();
		}

	private:
		std::unordered_map<AssetID, RefPtr<Asset> > m_assets;
	};
}
