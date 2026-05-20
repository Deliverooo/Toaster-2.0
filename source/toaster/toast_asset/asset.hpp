#pragma once

#include "toaster_macros.hpp"

#include "toast_lib/ptr.hpp"
#include "toast_lib/string.hpp"
#include "toast_lib/uuid.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster::asset
{
	using AssetID = UUID;
	constexpr AssetID c_invalidAssetID{UINT64_MAX}; // Maybe this should be different

	// I will add more types as I go on
	enum class EAssetType
	{
		eInvalid,
		eTexture2D,
		eTexture3D,
		eMesh
	};

	TST_API inline auto assetTypeToString(EAssetType p_asset_type) -> String
	{
		switch (p_asset_type)
		{
			case EAssetType::eInvalid: return "Invalid";
			case EAssetType::eTexture2D: return "Texture 2D";
			case EAssetType::eTexture3D: return "Texture 3D";
			case EAssetType::eMesh: return "Mesh";
		}
		return "";
	}

	TST_API inline auto assetTypeFromString(const String &p_asset_type) -> EAssetType
	{
		if (p_asset_type == "Invalid")
			return EAssetType::eInvalid;
		if (p_asset_type == "Texture 2D")
			return EAssetType::eTexture2D;
		if (p_asset_type == "Texture 3D")
			return EAssetType::eTexture3D;
		if (p_asset_type == "Mesh")
			return EAssetType::eMesh;
		return EAssetType::eInvalid;
	}

	class TST_API Asset
	{
	public:
		virtual ~Asset()
		{
		}

		auto operator==(const Asset &p_other) const -> bool
		{
			return id == p_other.id;
		}

		auto operator!=(const Asset &p_other) const -> bool
		{
			return id != p_other.id;
		}

		virtual auto getAssetType() const -> EAssetType = 0;

	protected:
		AssetID id{c_invalidAssetID};
	};

	using AssetHandle = RefPtr<Asset>;

	#define TST_ASSET(__type) public: static auto getStaticType() -> EAssetType {return EAssetType::e##__type;}\
							  virtual auto getAssetType() const -> EAssetType override {return getStaticType();} private:

	struct AssetMetadata
	{
		io::filesystem::Path path{}; // The path to the asset on disk
		EAssetType           type;
	};

	const AssetMetadata c_invalidAssetMetadata{};
}
