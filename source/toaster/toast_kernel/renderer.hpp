#pragma once

#include "render_command.hpp"

namespace toaster
{
	// Static interface class
	class Renderer final
	{
	public:
		static void submitQuad(const glm::vec3 &p_positon);
	};
}
