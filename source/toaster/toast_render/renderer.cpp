#include "renderer.hpp"

#include "globals.hpp"

#include <glm/ext/matrix_transform.hpp>

#include "toaster/toast_lib/logging.hpp"

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

		for (const auto &sm: p_mesh->getSubmeshes())
		{
			glm::mat4  transform = p_model_matrix * sm.transform;
			const auto material  = p_mesh->getMaterial(sm.materialIndex);

			mesh_shader->setUniform("u_Model", transform);
			material->use();

			RenderCommand::drawIndexedBaseVertex(p_mesh->getVertexArray(), sm.indexCount, sm.baseIndex, sm.baseVertex);
		}
	}

	void Renderer::renderSubmesh(const RefPtr<Mesh> &p_mesh, uint32 p_submesh_index, const glm::mat4 &p_view, const glm::mat4 &p_proj, const glm::mat4 &p_model_matrix)
	{
		auto mesh_shader = Globals::shaderLibrary()->get("Mesh");
		mesh_shader->bind();
		mesh_shader->setUniform("u_View", p_view);
		mesh_shader->setUniform("u_Proj", p_proj);

		const Submesh sm = p_mesh->getSubmeshes()[p_submesh_index];

		glm::mat4  transform = p_model_matrix * sm.transform;
		const auto material  = p_mesh->getMaterial(sm.materialIndex);

		mesh_shader->setUniform("u_Model", transform);
		material->use();

		RenderCommand::drawIndexedBaseVertex(p_mesh->getVertexArray(), sm.indexCount, sm.baseIndex, sm.baseVertex);
	}

	void Renderer::submitQuad(const glm::vec3 &p_positon)
	{
		submitGeometry(Globals::quadVertexArray(), Globals::shaderLibrary()->get("Quad"), glm::translate(glm::mat4{1.0f}, p_positon));
	}
}
