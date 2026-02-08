#pragma once

#include "render_command.hpp"
#include "shader.hpp"

namespace toaster
{
	// Static interface class
	class Renderer final
	{
	public:
		static void submitGeometry(const RefPtr<gpu::VertexArray> &p_vertex_array, const RefPtr<gpu::Shader> &p_shader, const glm::mat4 &p_model_matrix);

		static void submitQuad(const glm::vec3 &p_positon);
	};
}
