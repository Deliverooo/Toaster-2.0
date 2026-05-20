#include "asset_manager.hpp"

#include "texture_asset.hpp"
#include "toast_lib/buffer.hpp"
#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_project/project.hpp"

#include <yaml-cpp/yaml.h>

#include "mesh_asset.hpp"
#include "toast_render/mesh.hpp"

namespace toaster::asset
{
	AssetManager::AssetManager(Project *p_project) : m_project(p_project)
	{
	}

	AssetManager::AssetManager(Project *p_project, render::RenderContext *p_render_ctx) : m_project(p_project), m_renderCtx(p_render_ctx)
	{
	}

	auto AssetManager::getAsset(AssetID p_asset_id) -> AssetHandle
	{
		if (!isAssetIDValid(p_asset_id))
			return nullptr;

		if (isAssetLoaded(p_asset_id))
			return m_loadedAssets.at(p_asset_id);

		const auto &metadata{m_assetMetadataRegistry.at(p_asset_id)};
		AssetHandle asset_handle{s_assetImporters.at(metadata.type)(this, metadata)};
		m_loadedAssets[p_asset_id] = asset_handle;
		if (!asset_handle)
			return nullptr;

		return asset_handle;
	}

	auto AssetManager::isAssetIDValid(AssetID p_asset_id) const -> bool
	{
		return m_assetMetadataRegistry.contains(p_asset_id);
	}

	auto AssetManager::isAssetLoaded(AssetID p_asset_id) const -> bool
	{
		return m_loadedAssets.contains(p_asset_id);
	}

	auto AssetManager::getAssetMetadata(AssetID p_asset_id) const -> const AssetMetadata &
	{
		if (!isAssetIDValid(p_asset_id))
		{
			TST_PERMA_ASSERT_MSG(false, "Asset id is not valid, cannot retrieve metadata!");
			return m_assetMetadataRegistry.at(c_invalidAssetID);
		}
		return m_assetMetadataRegistry.at(p_asset_id);
	}

	auto AssetManager::addAssetMetadata(AssetID p_asset_id, const AssetMetadata &p_metadata) -> void
	{
		if (m_assetMetadataRegistry.contains(p_asset_id))
		{
			TST_PERMA_ASSERT_MSG(false, "Registry already contains the specified asset id");
			return;
		}
		m_assetMetadataRegistry[p_asset_id] = p_metadata;
	}

	auto AssetManager::serializeToFile(const io::filesystem::Path &p_path) const -> void
	{
		YAML::Emitter out{};

		out << YAML::BeginMap;
		out << YAML::Key << "AssetRegistry" << YAML::Value << YAML::BeginSeq;

		for (const auto &[id, metadata]: m_assetMetadataRegistry)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "AssetID" << YAML::Value << id;
			out << YAML::Key << "AssetPath" << YAML::Value << metadata.path.string();
			out << YAML::Key << "AssetType" << YAML::Value << assetTypeToString(metadata.type);
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		io::filesystem::writeFile(p_path, out.c_str());
	}

	auto AssetManager::deserializeFromFile(const io::filesystem::Path &p_path) -> void
	{
		const String yaml_string{io::filesystem::readFile(p_path)};
		YAML::Node   data{YAML::Load(yaml_string)};

		auto asset_registry_node{data["AssetRegistry"]};
		if (!asset_registry_node)
		{
			LOG_FATAL("Failed to deserialize asset registry: {}", p_path);
			return;
		}

		for (const auto &node: asset_registry_node)
		{
			auto &metadata{m_assetMetadataRegistry[static_cast<AssetID>(node["AssetID"].as<uint64>())]};
			metadata.path = node["AssetPath"].as<String>();
			metadata.type = assetTypeFromString(node["AssetType"].as<String>());
		}
	}

	auto AssetManager::printAssetRegistry() const -> void
	{
		for (const auto &[id, metadata]: m_assetMetadataRegistry)
		{
			LOG_INFO("AssetID: {}", (uint64)id);
			LOG_INFO("\tAssetPath: {}", metadata.path);
			LOG_INFO("\tAssetType: {}", assetTypeToString(metadata.type));
		}
	}

	auto AssetManager::importTexture2DAsset(const AssetManager *p_asm, const AssetMetadata &p_metadata) -> AssetHandle
	{
		io::filesystem::Path full_path{p_asm->m_project->getRootDirectory() / "resources" / p_metadata.path};
		LOG_INFO("Loading texture 2d asset: {}", full_path);
		gpu::TextureSpecInfo texture_spec_info{};
		Buffer               texture_data{gpu::util::loadTextureImage(full_path, texture_spec_info.format, texture_spec_info.width, texture_spec_info.height)};

		gpu::Texture2DHandle texture{p_asm->m_renderCtx->createGPU<gpu::VKTexture2D>(texture_spec_info, Buffer::copy(texture_data))};
		texture_data.release();
		return make_reference<Texture2DAsset>(texture).as<Asset>();
	}

	auto AssetManager::importTexture3DAsset(const AssetManager *p_asm, const AssetMetadata &p_metadata) -> AssetHandle
	{
		io::filesystem::Path full_path{p_asm->m_project->getRootDirectory() / "resources" / p_metadata.path};
		LOG_INFO("Loading texture 3d asset: {}", full_path);

		gpu::TextureSpecInfo texture_spec_info{};
		Buffer               texture_data{gpu::util::loadTextureImage(full_path, texture_spec_info.format, texture_spec_info.width, texture_spec_info.height)};
		gpu::Texture3DHandle texture{p_asm->m_renderCtx->createEnvironmentMap(texture_spec_info, Buffer::copy(texture_data))};
		texture_data.release();
		return make_reference<Texture3DAsset>(texture).as<Asset>();
	}

	auto AssetManager::importMeshAsset(const AssetManager *p_asm, const AssetMetadata &p_metadata) -> AssetHandle
	{
		io::filesystem::Path full_path{p_asm->m_project->getRootDirectory() / p_metadata.path};
		LOG_INFO("Loading mesh asset: {}", full_path);

		render::MeshHandle mesh{p_asm->m_renderCtx->create<render::MeshData>(full_path)};

		return make_reference<MeshAsset>(mesh).as<Asset>();
	}
}
