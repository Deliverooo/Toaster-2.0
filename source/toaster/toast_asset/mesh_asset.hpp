#pragma once

#include "asset.hpp"
#include "toast_render/mesh.hpp"

namespace toaster::asset
{
	class TST_API MeshAsset : public Asset
	{
		TST_ASSET(Mesh)
	public:
		MeshAsset(const render::MeshHandle &p_mesh);
		virtual ~MeshAsset() override = default;

		auto getMesh() const -> const render::MeshHandle &;

	private:
		render::MeshHandle m_mesh{nullptr};
	};
}
