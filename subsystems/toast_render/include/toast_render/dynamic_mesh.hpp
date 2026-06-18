#pragma once

#include "toast_render.hpp"

namespace toaster::render
{
	struct TST_RENDER_API Meshlet
	{
		uint32 vertexOffset{0u};
		uint32 vertexCount{0u};
		uint32 triangleOffset{0u};
		uint32 triangleCount{0u};
	};
}
