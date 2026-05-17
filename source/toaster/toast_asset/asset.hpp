#pragma once

#include "toast_lib/uuid.hpp"

namespace toaster::asset
{
	using AssetID = UUID;
	const AssetID c_invalidAssetID{UINT64_MAX};

	struct Asset
	{
		virtual ~Asset() = 0;

		auto operator==(const Asset &p_other) const -> bool
		{
			return id == p_other.id;
		}

		auto operator!=(const Asset &p_other) const -> bool
		{
			return id != p_other.id;
		}

		AssetID id{c_invalidAssetID};
	};
}
