#pragma once

#include "toast_lib/system_types.h"

namespace toaster::asset
{
	using AssetID = uint64;
	constexpr AssetID c_invalidAssetID{UINT64_MAX};

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
