#pragma once

#include "render_command.hpp"
#include "toaster/toast_gpu/shader.hpp"

#include "mesh.hpp"

namespace toaster
{
	// Static interface class
	class Renderer final
	{
	public:
		static void submitGeometry(const RefPtr<gpu::IVertexArray> &p_vertex_array, const RefPtr<gpu::IShader> &p_shader, const glm::mat4 &p_model_matrix);

		static void submitMesh(const RefPtr<Mesh> &p_mesh, const glm::mat4 &p_view, const glm::mat4 &p_proj, const glm::mat4 &p_model_matrix);
		static void renderSubmesh(const RefPtr<Mesh> &p_mesh, uint32 p_submesh_index, const glm::mat4 &p_view, const glm::mat4 &p_proj, const glm::mat4 &p_model_matrix);

		static void submitQuad(const glm::vec3 &p_positon);
	};
}
