#include "renderer.hpp"

#include "globals.hpp"

#include <openglhpp/opengl.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace toaster
{
	void Renderer::submitQuad(const glm::vec3 &p_positon)
	{
		glm::mat4 model = glm::translate(glm::mat4{1.0f}, p_positon);
		gpu::Globals::quadShader()->bind();

		gpu::Globals::quadShader()->setUniform("u_Model", model);

		gpu::Globals::quadVertexArray()->bind();
		gl::drawElements(gl::DrawMode::eTriangles, gpu::Globals::quadVertexArray()->getIndexBuffer()->getIndexCount(), gl::DataType::eUnsignedInt, nullptr);
	}
}
