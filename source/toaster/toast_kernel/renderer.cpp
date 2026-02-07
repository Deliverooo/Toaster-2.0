#include "renderer.hpp"

#include "globals.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace toaster
{
	void Renderer::submitQuad(const glm::vec3 &p_positon)
	{
		glm::mat4 model = glm::translate(glm::mat4{1.0f}, p_positon);
		gpu::Globals::quadShader()->bind();

		gpu::Globals::quadShader()->setUniform("u_Model", model);

		RenderCommand::drawIndexed(gpu::Globals::quadVertexArray());
	}
}
