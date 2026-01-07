#pragma once

#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "system_types.h"

#include "index_buffer.hpp"
#include "texture.hpp"
#include "vertex_buffer.hpp"

namespace toaster
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoord;
		glm::vec3 tangent;

		static vk::VertexInputBindingDescription getBindingDescription()
		{
			vk::VertexInputBindingDescription binding{};
			binding.binding   = 0;
			binding.stride    = sizeof(Vertex);
			binding.inputRate = vk::VertexInputRate::eVertex;
			return binding;
		}

		static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions()
		{
			std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions{};
			
			// Position
			attributeDescriptions[0].binding  = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].offset   = offsetof(Vertex, position);
			attributeDescriptions[0].format   = vk::Format::eR32G32B32Sfloat;

			// Normal
			attributeDescriptions[1].binding  = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].offset   = offsetof(Vertex, normal);
			attributeDescriptions[1].format   = vk::Format::eR32G32B32Sfloat;

			// TexCoord
			attributeDescriptions[2].binding  = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].offset   = offsetof(Vertex, texCoord);
			attributeDescriptions[2].format   = vk::Format::eR32G32Sfloat;

			// Tangent
			attributeDescriptions[3].binding  = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].offset   = offsetof(Vertex, tangent);
			attributeDescriptions[3].format   = vk::Format::eR32G32B32Sfloat;

			return attributeDescriptions;
		}
	};

	struct SubMesh
	{
		uint32 indexOffset;
		uint32 indexCount;
		uint32 vertexOffset;
	};

	class Mesh
	{
	public:
		Mesh() = default;
		~Mesh();

		bool loadFromFile(const std::string &filePath, gpu::GPUContext *gpuContext);
		
		void destroy();

		[[nodiscard]] std::shared_ptr<gpu::VertexBuffer> getVertexBuffer() const { return m_vertexBuffer; }
		[[nodiscard]] std::shared_ptr<gpu::IndexBuffer>  getIndexBuffer() const { return m_indexBuffer; }
		[[nodiscard]] uint32                             getIndexCount() const { return static_cast<uint32>(m_indices.size()); }
		[[nodiscard]] uint32                             getVertexCount() const { return static_cast<uint32>(m_vertices.size()); }
		[[nodiscard]] const std::vector<SubMesh> &       getSubMeshes() const { return m_subMeshes; }
		[[nodiscard]] bool                               isLoaded() const { return m_isLoaded; }


	private:
		void processNode(aiNode *node, const aiScene *scene);
		void processMesh(aiMesh *mesh, const aiScene *scene);

		void createVertexBuffer();
		void createIndexBuffer();

		std::filesystem::path m_path;
		std::filesystem::path m_directory; // Directory containing the mesh file

		gpu::GPUContext *m_gpuContext{nullptr};

		std::vector<Vertex>       m_vertices;
		std::vector<uint32>       m_indices;
		std::vector<SubMesh>      m_subMeshes;

		std::shared_ptr<gpu::VertexBuffer> m_vertexBuffer{nullptr};
		std::shared_ptr<gpu::IndexBuffer>  m_indexBuffer{nullptr};

		bool m_isLoaded{false};
		bool m_materialsLoaded{false};
	};
}
