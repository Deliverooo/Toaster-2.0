#pragma once

#include "material.hpp"
#include "shader.hpp"
#include "vertex_array.hpp"

namespace toaster::gpu
{
	class Globals
	{
	public:
		static void init();
		static void shutdown();

		static RefPtr<Shader> quadShader();

		static RefPtr<VertexArray> quadVertexArray();

	};
}
