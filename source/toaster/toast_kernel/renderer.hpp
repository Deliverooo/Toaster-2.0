#pragma once

#include "render_command.hpp"
#include "shader.hpp"

#include "mesh.hpp"

namespace toaster
{
	// Static interface class
	class Renderer final
	{
	public:
		static void submitGeometry(const RefPtr<gpu::VertexArray> &p_vertex_array, const RefPtr<gpu::Shader> &p_shader, const glm::mat4 &p_model_matrix);

		static void submitMesh(const RefPtr<Mesh> &p_mesh, const glm::mat4 &p_view, const glm::mat4 &p_proj, const glm::mat4 &p_model_matrix);

		static void submitQuad(const glm::vec3 &p_positon);
	};
}
