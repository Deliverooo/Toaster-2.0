#pragma once

#include "mesh.hpp"
#include "shader.hpp"

namespace toaster
{
	// Static interface class
	class Renderer
	{
	public:
		static void submitQuad(const glm::vec3 &p_positon);
	};
}
