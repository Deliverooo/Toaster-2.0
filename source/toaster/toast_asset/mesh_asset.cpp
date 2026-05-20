#include "mesh_asset.hpp"

namespace toaster::asset
{
	MeshAsset::MeshAsset(const render::MeshHandle &p_mesh) : m_mesh(p_mesh)
	{
	}

	auto MeshAsset::getMesh() const -> const render::MeshHandle &
	{
		return m_mesh;
	}
}
