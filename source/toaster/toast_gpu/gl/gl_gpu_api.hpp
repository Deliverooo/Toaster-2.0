#pragma once

#include "../gpu_api.hpp"

namespace toaster::gpu
{
	class GLGPUAPI final : public IGPUAPI
	{
	public:
		GLGPUAPI();

		void clearColour(float32 p_r, float32 p_g, float32 p_b, float32 p_a) override;
		void clear() override;

		void setEnableFaceCulling(bool p_enable) override;
		void setFaceCullMode(ECullMode p_cull_mode) override;

		void setViewport(int32 p_x, int32 p_y, int32 p_width, int32 p_height) override;

		void drawIndexed(const RefPtr<IVertexArray> &p_vertex_array, uint32 p_index_count) override;
		void drawIndexedBaseVertex(const RefPtr<IVertexArray> &p_vertex_array, uint32 p_index_count, uint32 p_base_index, uint32 p_base_vertex) override;

	};
}
