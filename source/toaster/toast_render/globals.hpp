#pragma once

#include "toast_gpu/vertex_array.hpp"
#include "toast_render/shader_library.hpp"

namespace toaster
{
	class Globals final
	{
	public:
		static void init();
		static void shutdown();

		static const RefPtr<ShaderLibrary> &shaderLibrary();

		static RefPtr<gpu::IVertexArray> quadVertexArray();
		static RefPtr<gpu::IVertexArray> fullscreenQuadVertexArray();
	};
}
