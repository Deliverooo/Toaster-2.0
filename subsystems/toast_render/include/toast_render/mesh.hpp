#pragma once

#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

#include "material.hpp"

namespace toaster::render
{
	struct TST_RENDER_API MeshVertex
	{
		tsm::float3 position{0.0f};
		tsm::float3 normal{0.0f};
		tsm::float3 tangent{0.0f};
		tsm::float3 bitangent{0.0f};
		tsm::float2 texCoord{0.0f};
	};

	struct TST_RENDER_API BoneInfluence
	{
		tsm::uint4  boneIds{0u};
		tsm::float4 boneWeights{0.0f};
	};

	struct TST_RENDER_API BoneInfo
	{
		tsm::float4x4 inverseBindPose{1.0f};
		uint32        boneIndex{0u};
	};

	struct TST_RENDER_API Submesh
	{
		String name{};

		tsm::float4x4 transform{1.0f};
		tsm::float4x4 localTransform{1.0f};

		uint32 baseVertex{0u};
		uint32 baseIndex{0u};
		uint32 materialIndex{0u};
		uint32 indexCount{0u};
		uint32 vertexCount{0u};
	};

	struct TST_RENDER_API MeshNode
	{
		String        name{};
		tsm::float4x4 localTransform{1.0f};

		std::vector<uint32> children;
		std::vector<uint32> submeshes;
		uint32              parent{UINT32_MAX};
	};

	struct TST_RENDER_API MeshMaterialData
	{
		auto setAlbedoMap(const gpu::Texture2DHandle &p_albedo_map) -> void
		{
			albedoMap = p_albedo_map;
			material->setTexture("u_AlbedoTexture", albedoMap);
		}

		auto setNormalMap(const gpu::Texture2DHandle &p_normal_map) -> void
		{
			normalMap = p_normal_map;
			material->setTexture("u_NormalTexture", normalMap);
		}

		String               name{};
		MaterialHandle       material{nullptr};
		gpu::Texture2DHandle albedoMap{nullptr};
		gpu::Texture2DHandle normalMap{nullptr};
	};

	class TST_RENDER_API MaterialList
	{
	public:
		MaterialList(RenderContext *p_render_ctx);

		auto               addMaterial(uint32 p_index, const gpu::ShaderHandle &p_shader, const String &p_name) -> MeshMaterialData &;
		[[nodiscard]] auto hasMaterial(uint32 p_index) const -> bool;
		auto               getMaterial(uint32 p_index) -> MeshMaterialData &;
		[[nodiscard]] auto getMaterial(uint32 p_index) const -> const MeshMaterialData &;

		auto               begin() { return m_materialDatas.begin(); }
		auto               end() { return m_materialDatas.begin(); }
		[[nodiscard]] auto begin() const { return m_materialDatas.begin(); }
		[[nodiscard]] auto end() const { return m_materialDatas.begin(); }

		auto data() -> std::unordered_map<uint32, MeshMaterialData> & { return m_materialDatas; }

	private:
		NonOwningPtr<RenderContext> m_renderCtx{nullptr};

		std::unordered_map<uint32, MeshMaterialData> m_materialDatas;
	};

	class TST_RENDER_API MeshData
	{
	public:
		MeshData(RenderContext *p_render_ctx, const io::filesystem::Path &p_path);
		MeshData(RenderContext *p_render_ctx, const io::filesystem::Path &p_path, const gpu::ShaderHandle &p_shader);

		auto getVertexBuffer() const -> const gpu::VertexBufferHandle &;
		auto getIndexBuffer() const -> const gpu::IndexBufferHandle &;

		auto getMaterials() -> MaterialList &;
		auto getMaterials() const -> const MaterialList &;
		auto getSubmeshes() const -> const std::vector<Submesh> &;

		auto getVertices() const -> const std::vector<MeshVertex> &;
		auto getIndices() const -> const std::vector<uint32> &;

		auto getFilepath() const -> const io::filesystem::Path &;

	private:
		auto _traverseNodes(void *p_assimp_node, uint32 p_node_index, const tsm::float4x4 &p_parent_transform, uint32 p_level) -> void;
		auto _createMaterial(void *p_mat, uint32 p_mat_index, const io::filesystem::Path &p_parent_path) -> void;

		NonOwningPtr<RenderContext> m_renderCtx{nullptr};

		io::filesystem::Path m_path;

		std::vector<Submesh>  m_submeshes;
		std::vector<MeshNode> m_nodes;

		std::vector<MeshVertex> m_vertices;
		std::vector<uint32>     m_indices;

		gpu::VertexBufferHandle m_vertexBuffer{nullptr};
		gpu::IndexBufferHandle  m_indexBuffer{nullptr};

		MaterialList m_materials;
	};

	using MeshHandle = RefPtr<MeshData>;

	class TST_RENDER_API StaticMesh
	{
	public:
		StaticMesh(RenderContext *p_render_ctx, const RefPtr<MeshData> &p_mesh_data);

		auto getMeshData() const -> const RefPtr<MeshData> &;

	private:
		NonOwningPtr<RenderContext> m_renderCtx{nullptr};

		RefPtr<MeshData> m_meshData{nullptr};
	};

	class TST_RENDER_API DynamicMesh
	{
	public:
		DynamicMesh(RenderContext *p_render_ctx, const RefPtr<MeshData> &p_mesh_data);

		auto getMeshData() const -> const RefPtr<MeshData> &;

	private:
		NonOwningPtr<RenderContext> m_renderCtx{nullptr};

		RefPtr<MeshData> m_meshData{nullptr};
	};
}
