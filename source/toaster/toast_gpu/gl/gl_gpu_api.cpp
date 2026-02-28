#include "gl_gpu_api.hpp"

#include <openglhpp/opengl.hpp>

namespace toaster::gpu
{
	GLGPUAPI::GLGPUAPI()
	{
		gl::enable(gl::Capability::eBlend);
		gl::blendFunc(gl::BlendFunc::eSrcAlpha, gl::BlendFunc::eOneMinusSrcAlpha);

		gl::enable(gl::Capability::eDepthTest);
		// gl::enable(gl::Capability::eCullFace);
	}

	void GLGPUAPI::clearColour(const glm::vec4 &p_colour)
	{
		gl::clearColor(p_colour.r, p_colour.g, p_colour.b, p_colour.a);
	}

	void GLGPUAPI::clear()
	{
		gl::clear(gl::ClearMaskBits::eColor | gl::ClearMaskBits::eDepth | gl::ClearMaskBits::eStencil);
	}

	void GLGPUAPI::setEnableFaceCulling(bool p_enable)
	{
		if (p_enable)
		{
			gl::enable(gl::Capability::eCullFace);
		}
		else
		{
			gl::disable(gl::Capability::eCullFace);
		}
	}

	void GLGPUAPI::setFaceCullMode(CullMode p_cull_mode)
	{
		switch (p_cull_mode)
		{
			case CullMode::eFront: gl::cullFace(gl::CullMode::eFront);
				break;
			case CullMode::eBack: gl::cullFace(gl::CullMode::eBack);
				break;
			case CullMode::eFrontAndBack: gl::cullFace(gl::CullMode::eFrontAndBack);
				break;
		}
	}

	void GLGPUAPI::setViewport(const glm::vec4 &p_viewport)
	{
		gl::viewport(static_cast<gl::Int>(p_viewport.x), static_cast<gl::Int>(p_viewport.y), static_cast<gl::Int>(p_viewport.z), static_cast<gl::Int>(p_viewport.w));
	}

	void GLGPUAPI::drawIndexed(const RefPtr<VertexArray> &p_vertex_array)
	{
		p_vertex_array->bind();
		gl::drawElements(gl::DrawMode::eTriangles, static_cast<int32>(p_vertex_array->getIndexBuffer()->getIndexCount()), gl::DataType::eUnsignedInt, nullptr);
	}

	void GLGPUAPI::drawIndexedBaseVertex(const RefPtr<VertexArray> &p_vertex_array, uint32 p_index_count, uint32 p_base_index, uint32 p_base_vertex)
	{
		p_vertex_array->bind();
		gl::drawElementsBaseVertex(gl::DrawMode::eTriangles, static_cast<int32>(p_vertex_array->getIndexBuffer()->getIndexCount()), gl::DataType::eUnsignedInt,
								   reinterpret_cast<gl::Void *>(p_base_index * sizeof(uint32)), static_cast<gl::Int>(p_base_vertex));
	}
}
