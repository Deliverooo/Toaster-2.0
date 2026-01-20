#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "system_types.h"
#include "io/filesystem.hpp"

#include "vertex_array.hpp"

namespace toaster::gpu
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		glm::vec2 texCoords;
	};

	class Mesh
	{
	public:

	private:
		io::filesystem::Path m_filepath;

		std::vector<Vertex> m_vertices;
		std::vector<uint32> m_indices;

		std::shared_ptr<VertexArray> m_vao;

		Material m_material;

	};
}
