#pragma once

#include "toaster/toast_gpu/vertex_array.hpp"
#include "toaster/toast_render/shader_library.hpp"

namespace toaster
{
	class Globals final
	{
	public:
		static void init();
		static void shutdown();

		static const RefPtr<ShaderLibrary> &shaderLibrary();

		static RefPtr<gpu::IVertexArray> quadVertexArray();
	};
}
