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

	void Renderer::submitMesh(const RefPtr<Mesh> &p_mesh, const glm::mat4 &p_view, const glm::mat4 &p_proj, const glm::mat4 &p_model_matrix)
	{
		auto mesh_shader = Globals::shaderLibrary()->get("Mesh");
		mesh_shader->bind();
		mesh_shader->setUniform("u_View", p_view);
		mesh_shader->setUniform("u_Proj", p_proj);
		mesh_shader->setUniform("u_Model", p_model_matrix);

		p_mesh->getMaterial()->use();
		RenderCommand::drawIndexed(p_mesh->getVertexArray());
	}

	void Renderer::submitQuad(const glm::vec3 &p_positon)
	{
		submitGeometry(Globals::quadVertexArray(), Globals::shaderLibrary()->get("Quad"), glm::translate(glm::mat4{1.0f}, p_positon));
	}
}
