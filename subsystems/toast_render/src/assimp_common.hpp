#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "toast_lib/system_types.h"

static constexpr uint32 s_MeshImportFlags{
	aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_OptimizeMeshes |
	aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_ValidateDataStructure | aiProcess_GlobalScale | aiProcess_FlipUVs
};

namespace toaster::render
{
	inline auto mat4FromAIMatrix4x4(const aiMatrix4x4 &matrix) -> Dx::XMFLOAT4X4
	{
		Dx::XMFLOAT4X4 result;
		Dx::XMStoreFloat4x4(&result, Dx::XMMatrixTranspose(Dx::XMLoadFloat4x4(reinterpret_cast<const Dx::XMFLOAT4X4 *>(&matrix))));
		return result;
	}
}
