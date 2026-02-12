#include "gl/gl_gpu_api.hpp"

#include <openglhpp/opengl.hpp>

namespace toaster::gpu
{
	GLGPUAPI::GLGPUAPI()
	{
		gl::enable(gl::Capability::eBlend);
		gl::blendFunc(gl::BlendFunc::eSrcAlpha, gl::BlendFunc::eOneMinusSrcAlpha);

		gl::enable(gl::Capability::eDepthTest);
		gl::enable(gl::Capability::eCullFace);
	}

	void GLGPUAPI::clearColour(const glm::vec4 &p_colour)
	{
		gl::clearColor(p_colour.r, p_colour.g, p_colour.b, p_colour.a);
	}

	void GLGPUAPI::clear()
	{
		gl::clear(gl::ClearMaskBits::eColor | gl::ClearMaskBits::eDepth | gl::ClearMaskBits::eStencil);
	}

	void GLGPUAPI::setViewport(const glm::vec4 &p_viewport)
	{
		gl::viewport(p_viewport.x, p_viewport.y, p_viewport.z, p_viewport.w);
	}

	void GLGPUAPI::drawIndexed(const RefPtr<VertexArray> &p_vertex_array)
	{
		p_vertex_array->bind();
		gl::drawElements(gl::DrawMode::eTriangles, static_cast<int32>(p_vertex_array->getIndexBuffer()->getIndexCount()), gl::DataType::eUnsignedInt, nullptr);
	}
}
