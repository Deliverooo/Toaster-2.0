#pragma once

#include "shader.hpp"
#include "framebuffer.hpp"
#include "vertex_buffer_layout.hpp"

namespace tst::gpu
{
	struct PipelineSpecInfo
	{
		RefPtr<Shader> shader;
	};
}
