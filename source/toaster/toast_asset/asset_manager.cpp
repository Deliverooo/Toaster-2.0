#include "asset_manager.hpp"

namespace toaster::asset
{
	AssetManager::AssetManager()
	{
	}

	auto AssetManager::getAsset(AssetID p_asset_id) const -> RefPtr<Asset>
	{
		return m_assets.at(p_asset_id);
	}
}
