#pragma once

#include <glm/glm.hpp>

#include "toast_gpu/vertex_array.hpp"

namespace toaster
{
	class RenderCommand final
	{
	public:
		static void clearColour(float32 p_r, float32 p_g, float32 p_b, float32 p_a);
		static void clear();

		static void setViewport(int32 p_x, int32 p_y, int32 p_width, int32 p_height);

		static void drawIndexed(const RefPtr<gpu::IVertexArray> &p_vertex_array, uint32 p_index_count = 0);
		static void drawIndexedBaseVertex(const RefPtr<gpu::IVertexArray> &p_vertex_array, uint32 p_index_count, uint32 p_base_index, uint32 p_base_vertex);

	private:
		// We only want the application to be the one initializing it
		static void init();
		friend class Application;
	};
}
