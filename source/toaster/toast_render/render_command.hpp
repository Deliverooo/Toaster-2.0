#pragma once

#include <glm/glm.hpp>

#include "toast_gpu/vertex_array.hpp"

namespace toaster
{
	class RenderCommand final
	{
	public:
		static void clearColour(const glm::vec4 &p_colour);
		static void clear();

		static void setViewport(const glm::vec4 &p_viewport);

		static void drawIndexed(const RefPtr<gpu::IVertexArray> &p_vertex_array, uint32 p_index_count = 0);
		static void drawIndexedBaseVertex(const RefPtr<gpu::IVertexArray> &p_vertex_array, uint32 p_index_count, uint32 p_base_index, uint32 p_base_vertex);

	private:
		// We only want the application to be the one initializing it
		static void init();
		friend class Application;
	};
}
