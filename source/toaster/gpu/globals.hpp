#pragma once

#include "shader.hpp"
#include "vertex_array.hpp"

namespace toaster::gpu
{
	class Globals final
	{
	public:
		static void init();
		static void shutdown();

		static RefPtr<Shader> quadShader();

		static RefPtr<VertexArray> quadVertexArray();
	};
}
