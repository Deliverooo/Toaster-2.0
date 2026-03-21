#pragma once

#include "toaster/toast_gpu/gpu_api.hpp"

namespace toaster::gpu
{
	class GLGPUAPI final : public IGPUAPI
	{
	public:
		GLGPUAPI();

		void clearColour(const glm::vec4 &p_colour) override;
		void clear() override;

		void setEnableFaceCulling(bool p_enable) override;
		void setFaceCullMode(ECullMode p_cull_mode) override;

		void setViewport(const glm::vec4 &p_viewport) override;

		void drawIndexed(const RefPtr<IVertexArray> &p_vertex_array, uint32 p_index_count) override;
		void drawIndexedBaseVertex(const RefPtr<IVertexArray> &p_vertex_array, uint32 p_index_count, uint32 p_base_index, uint32 p_base_vertex) override;

	};
}
