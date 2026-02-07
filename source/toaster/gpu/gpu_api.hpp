#pragma once

#include <glm/glm.hpp>

#include "vertex_array.hpp"

namespace toaster::gpu
{
	class GPUAPI
	{
	public:
		static std::unique_ptr<GPUAPI> create();

		virtual ~GPUAPI() = default;

		virtual void clearColour(const glm::vec4 &p_colour) = 0;
		virtual void clear() = 0;

		virtual void setViewport(const glm::vec4 &p_viewport) = 0;

		virtual void drawIndexed(const RefPtr<VertexArray> &p_vertex_array) = 0;
	};
}
