#include "renderer.hpp"

#include "globals.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace toaster
{
	void Renderer::submitGeometry(const RefPtr<gpu::VertexArray> &p_vertex_array, const RefPtr<gpu::Shader> &p_shader, const glm::mat4 &p_model_matrix)
	{
		p_shader->bind();
		p_shader->setUniform("u_Model", p_model_matrix);
		RenderCommand::drawIndexed(p_vertex_array);
	}

	void Renderer::submitQuad(const glm::vec3 &p_positon)
	{
		submitGeometry(gpu::Globals::quadVertexArray(), gpu::Globals::quadShader(), glm::translate(glm::mat4{1.0f}, p_positon));
	}
}
