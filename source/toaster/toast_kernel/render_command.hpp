#pragma once

#include <glm/glm.hpp>

#include "vertex_array.hpp"

namespace toaster
{
	class RenderCommand final
	{
	public:
		static void clearColour(const glm::vec4 &p_colour);
		static void clear();

		static void setViewport(const glm::vec4 &p_viewport);

		static void drawIndexed(const RefPtr<gpu::VertexArray> &p_vertex_array);


	private:
		// We only want the application to be the one initializing it
		static void init();
		friend class Application;
	};
}
