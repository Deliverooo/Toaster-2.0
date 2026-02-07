#pragma once

#include "gpu_api.hpp"

namespace toaster::gpu
{
	class GLGPUAPI final : public GPUAPI
	{
	public:
		GLGPUAPI() = default;

		void clearColour(const glm::vec4 &p_colour) override;
		void clear() override;

		void setViewport(const glm::vec4 &p_viewport) override;

		void drawIndexed(const RefPtr<VertexArray> &p_vertex_array) override;
	};
}
