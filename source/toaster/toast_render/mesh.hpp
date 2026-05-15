#pragma once

#include <glm/glm.hpp>
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"
#include "toast_lib/io/filesystem.hpp"

#include "material.hpp"

namespace toaster::render
{
	struct TST_API MeshVertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		glm::vec2 texCoord;
	};

	struct TST_API Submesh
	{
		String name{};

		glm::mat4 transform{1.0f};
		glm::mat4 localTransform{1.0f};

		uint32 baseVertex{0u};
		uint32 baseIndex{0u};
		uint32 materialIndex{0u};
		uint32 indexCount{0u};
		uint32 vertexCount{0u};
	};

	struct TST_API MeshNode
	{
		String    name{};
		glm::mat4 localTransform{1.0f};

		std::vector<uint32> children;
		std::vector<uint32> submeshes;
		uint32              parent{UINT32_MAX};
	};

	struct TST_API MeshMaterialData
	{
		MaterialHandle       material{nullptr};
		gpu::Texture2DHandle albedoMap{nullptr};
		gpu::Texture2DHandle normalMap{nullptr};
	};

	class TST_API Mesh
	{
	public:
		Mesh(RenderContext *p_render_ctx, const io::filesystem::Path &p_path, const gpu::ShaderHandle &p_shader);
		~Mesh();

		auto getVertexBuffer() const -> const gpu::VertexBufferHandle &;
		auto getIndexBuffer() const -> const gpu::IndexBufferHandle &;

		auto getMaterialDatas() const -> const std::vector<MeshMaterialData> &;
		auto getSubmeshes() const -> const std::vector<Submesh> &;

		auto getVertices() const -> const std::vector<MeshVertex> &;
		auto getIndices() const -> const std::vector<uint32> &;

		auto getFilepath() const -> const io::filesystem::Path &;

	private:
		auto _traverseNodes(void *p_assimp_node, uint32 p_node_index, const glm::mat4 &p_parent_transform, uint32 p_level) -> void;

		NonOwningPtr<RenderContext> m_renderCtx{nullptr};

		io::filesystem::Path m_path;

		std::vector<Submesh>  m_submeshes;
		std::vector<MeshNode> m_nodes;

		std::vector<MeshVertex> m_vertices;
		std::vector<uint32>     m_indices;

		gpu::VertexBufferHandle m_vertexBuffer{nullptr};
		gpu::IndexBufferHandle  m_indexBuffer{nullptr};

		std::vector<MeshMaterialData> m_materialDatas;
	};

	using MeshHandle = RefPtr<Mesh>;
}
