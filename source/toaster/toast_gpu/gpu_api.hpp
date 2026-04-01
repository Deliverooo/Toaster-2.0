#pragma once

#include <glm/glm.hpp>

#include "vertex_array.hpp"

namespace toaster::gpu
{
	enum class ECullMode
	{
		eFront, eBack, eFrontAndBack
	};

	class IGPUAPI
	{
	public:
		static std::unique_ptr<IGPUAPI> create();

		virtual ~IGPUAPI() = default;

		virtual void clearColour(float32 p_r, float32 p_g, float32 p_b, float32 p_a) = 0;
		virtual void clear() = 0;

		virtual void setEnableFaceCulling(bool p_enable) = 0;
		virtual void setFaceCullMode(ECullMode p_cull_mode) = 0;

		virtual void setViewport(int32 p_x, int32 p_y, int32 p_width, int32 p_height) = 0;

		virtual void drawIndexed(const RefPtr<IVertexArray> &p_vertex_array, uint32 p_index_count) = 0;
		virtual void drawIndexedBaseVertex(const RefPtr<IVertexArray> &p_vertex_array, uint32 p_index_count, uint32 p_base_index, uint32 p_base_vertex) = 0;
	};
}
