#pragma once

#include "asset.hpp"
#include "toast_lib/map.hpp"
#include "toast_render/render_context.hpp"

namespace toaster
{
	class Project; // Used to resolve the file paths
}

namespace toaster::asset
{
	class AssetImporter;

	class TST_API AssetManager
	{
	public:
		AssetManager(Project *p_project);
		AssetManager(Project *p_project, render::RenderContext *p_render_ctx);

		[[nodiscard]] auto getAsset(AssetID p_asset_id) -> AssetHandle;

		template<typename TAsset>
		[[nodiscard]] auto getAsset(AssetID p_asset_id) -> RefPtr<TAsset>
		{
			return getAsset(p_asset_id).as<TAsset>();
		}

		template<typename TFunc>
		auto removeAssetIf(TFunc &&p_func) -> void
		{
			std::erase_if(m_assetMetadataRegistry, std::forward<TFunc>(p_func));
			// std::ranges::remove_if(m_loadedAssets, std::forward<TFunc>(p_func));
		}

		// Returns true if the asset id has an associated AssetMetadata struct. This will not necessarily mean that the asset actually exists in memory tho
		auto isAssetIDValid(AssetID p_asset_id) const -> bool;
		// Returns true if the asset id actually maps to an asset currently loaded into memory. if true, isAssetIDValid(...) will also be true
		auto isAssetLoaded(AssetID p_asset_id) const -> bool;

		auto getAssetMetadata(AssetID p_asset_id) const -> const AssetMetadata &;
		auto addAssetMetadata(AssetID p_asset_id, const AssetMetadata &p_metadata) -> void;

		auto getAssetMetadataRegistry() const -> const std::unordered_map<AssetID, AssetMetadata> &;
		auto removeAsset(AssetID p_asset_id) -> void;

		auto serializeToFile(const io::filesystem::Path &p_path) const -> void;
		auto deserializeFromFile(const io::filesystem::Path &p_path) -> void;

		auto printAssetRegistry() const -> void;

	private:
		NonOwningPtr<Project>               m_project{nullptr};
		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

		std::unordered_map<AssetID, AssetMetadata> m_assetMetadataRegistry;
		std::unordered_map<AssetID, AssetHandle>   m_loadedAssets; // The assets that have been loaded from a file into the application

		static auto importTexture2DAsset(const AssetManager *p_asm, const AssetMetadata &p_metadata) -> AssetHandle;
		static auto importTexture3DAsset(const AssetManager *p_asm, const AssetMetadata &p_metadata) -> AssetHandle;
		static auto importMeshAsset(const AssetManager *p_asm, const AssetMetadata &p_metadata) -> AssetHandle;

		// Every asset type has an associated AssetImporter derivation
		using ImportAssetFn = AssetHandle(*)(const AssetManager *, const AssetMetadata &);
		static constexpr MapConstexpr<EAssetType, ImportAssetFn, 3> s_assetImporters{
			{EAssetType::eTexture2D, importTexture2DAsset},
			{EAssetType::eTexture3D, importTexture3DAsset},
			{EAssetType::eMesh, importMeshAsset}
		};
	};
}
