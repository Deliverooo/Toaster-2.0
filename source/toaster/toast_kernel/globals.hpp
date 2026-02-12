#pragma once

#include "shader_library.hpp"
#include "vertex_array.hpp"

namespace toaster
{
	class Globals final
	{
	public:
		static void init();
		static void shutdown();

		static const RefPtr<ShaderLibrary> &shaderLibrary();

		static RefPtr<gpu::VertexArray> quadVertexArray();
	};
}
