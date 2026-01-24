#pragma once

#include "material.hpp"
#include "shader.hpp"

namespace toaster::gpu
{
	class Globals
	{
	public:
		static void init();
		static void shutdown();

		static RefPtr<Shader> defaultShader();
		static RefPtr<Material> defaultMaterial();
	};
}
