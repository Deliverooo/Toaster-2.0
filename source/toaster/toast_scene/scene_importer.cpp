#include "scene_importer.hpp"

#include "entity.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "toast_lib/math/math_matrix.hpp"

namespace toaster
{
	static constexpr uint32 s_MeshImportFlags{
		aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_OptimizeMeshes |
		aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_ValidateDataStructure | aiProcess_GlobalScale | aiProcess_FlipUVs
	};

	auto mat4FromAIMatrix4x4(const aiMatrix4x4 &matrix) -> glm::mat4
	{
		glm::mat4 result;
		result[0][0] = matrix.a1;
		result[1][0] = matrix.a2;
		result[2][0] = matrix.a3;
		result[3][0] = matrix.a4;
		result[0][1] = matrix.b1;
		result[1][1] = matrix.b2;
		result[2][1] = matrix.b3;
		result[3][1] = matrix.b4;
		result[0][2] = matrix.c1;
		result[1][2] = matrix.c2;
		result[2][2] = matrix.c3;
		result[3][2] = matrix.c4;
		result[0][3] = matrix.d1;
		result[1][3] = matrix.d2;
		result[2][3] = matrix.d3;
		result[3][3] = matrix.d4;
		return result;
	}

	SceneImporter::SceneImporter(Scene *p_scene) : m_scene(p_scene)
	{
		m_importer = new Assimp::Importer{};
	}

	SceneImporter::~SceneImporter()
	{
		delete static_cast<Assimp::Importer *>(m_importer);
	}

	auto SceneImporter::importFromFile(const io::filesystem::Path &p_path) -> Entity
	{
		const auto importer{static_cast<Assimp::Importer *>(m_importer)};

		const aiScene *scene = importer->ReadFile(p_path.string(), s_MeshImportFlags);
		if (!scene)
		{
			LOG_ERROR("Failed to load scene file: {0}", p_path.string());
			TST_ASSERT(false);
		}

		Entity root_entity{m_scene->createEntity(scene->mName.data)};
		auto & mesh_comp{root_entity.addComponent<MeshComponent>()};
		mesh_comp.mesh = m_scene->m_renderCtx->create<render::MeshData>(p_path, scene);

		{
			auto &          transform_comp{root_entity.getComponent<TransformComponent>()};
			const glm::mat4 root_transform{mat4FromAIMatrix4x4(scene->mRootNode->mTransformation)};
			tsm::decomposeTransform(root_transform, transform_comp.translation, transform_comp.orientation, transform_comp.scale);
		}

		const auto &submeshes{mesh_comp.mesh->getSubmeshes()};

		uint32 submesh_index{0u};
		for (const auto &submesh: submeshes)
		{
			Entity submesh_entity{m_scene->createEntity(submesh.name)};
			submesh_entity.setParent(root_entity);

			auto &submesh_component{submesh_entity.addComponent<SubmeshComponent>()};
			submesh_component.submeshIndex = submesh_index;
			submesh_component.mesh         = mesh_comp.mesh;

			auto &transform_comp{submesh_entity.getComponent<TransformComponent>()};
			tsm::decomposeTransform(submesh.localTransform, transform_comp.translation, transform_comp.orientation, transform_comp.scale);

			++submesh_index;
		}

		for (uint32 i{0u}; i < scene->mNumLights; ++i)
		{
			aiLight *light{scene->mLights[i]};

			Entity light_entity{m_scene->createEntity(light->mName.data)};
			auto & tc{light_entity.getComponent<TransformComponent>()};
			tc.translation = {light->mPosition.x, light->mPosition.y, light->mPosition.z};
			tc.orientation = glm::quat{glm::vec3{light->mDirection.x, light->mDirection.y, light->mDirection.z}};

			switch (light->mType)
			{
				case aiLightSource_POINT:
				{
					auto &plc{light_entity.addComponent<PointLightComponent>()};
					plc.radiance = {light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b};
					plc.radiance /= 100.0f;
					break;
				}
				case aiLightSource_DIRECTIONAL:
				{
					auto &dlc{light_entity.addComponent<DirectionalLightComponent>()};
					dlc.radiance = {light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b};
					break;
				}
				default:
				{
					break;
				}
			}
		}

		return root_entity;
	}
}
