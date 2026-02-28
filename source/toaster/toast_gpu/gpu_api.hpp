#pragma once

#include <glm/glm.hpp>

#include "vertex_array.hpp"

namespace toaster::gpu
{
	enum class CullMode
	{
		eFront,
		eBack,
		eFrontAndBack
	};

	class GPUAPI
	{
	public:
		static std::unique_ptr<GPUAPI> create();

		virtual ~GPUAPI() = default;

		virtual void clearColour(const glm::vec4 &p_colour) = 0;
		virtual void clear() = 0;

		virtual void setEnableFaceCulling(bool p_enable) = 0;
		virtual void setFaceCullMode(CullMode p_cull_mode) = 0;

		virtual void setViewport(const glm::vec4 &p_viewport) = 0;

		virtual void drawIndexed(const RefPtr<VertexArray> &p_vertex_array) = 0;
		virtual void drawIndexedBaseVertex(const RefPtr<VertexArray> &p_vertex_array, uint32 p_index_count, uint32 p_base_index, uint32 p_base_vertex) = 0;
	};
}
