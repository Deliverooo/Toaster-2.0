#include "renderer.hpp"

#include "globals.hpp"

#include <glm/ext/matrix_transform.hpp>

#include "toast_lib/logging.hpp"

namespace toaster
{
	void Renderer::submitGeometry(const RefPtr<gpu::IVertexArray> &p_vertex_array, const RefPtr<gpu::IShader> &p_shader, const glm::mat4 &p_model_matrix)
	{
		p_shader->bind();
		p_shader->setUniform("u_Model", p_model_matrix);
		RenderCommand::drawIndexed(p_vertex_array);
	}

	void Renderer::submitMesh(const RefPtr<Mesh> &p_mesh, const glm::mat4 &p_transform)
	{
		for (auto &submesh: p_mesh->getSubmeshes())
		{
			auto material = p_mesh->getMaterials()[submesh.materialIndex];
			auto shader   = material->getShader();
			material->use();  // This binds the shader and uploads uniforms

			// Note: For mesh rendering, prefer using material->set() for type-safe uniform setting
			// But direct shader->setUniform() also works if shader is already bound via material->use()
			shader->setUniform("u_Model", glm::mat4{p_transform * submesh.transform});

			RenderCommand::drawIndexedBaseVertex(p_mesh->getVertexArray(), submesh.indexCount, submesh.baseIndex, submesh.baseVertex);
		}
	}

	void Renderer::submitFullscreenQuad(const RefPtr<Material> &p_material)
	{
		p_material->use();
		RenderCommand::drawIndexed(Globals::fullscreenQuadVertexArray());
	}

	void Renderer::submitQuad(const glm::vec3 &p_positon)
	{
		submitGeometry(Globals::quadVertexArray(), Globals::shaderLibrary()->get("Quad"), glm::translate(glm::mat4{1.0f}, p_positon));
	}
}
