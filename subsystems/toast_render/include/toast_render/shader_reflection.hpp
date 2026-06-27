#pragma once

#include "toast_render.hpp"
#include "toast_gpu/vk/vk_shader.hpp"

namespace toaster::render
{
	TST_RENDER_API auto reflectShader(const gpu::DynamicShader &p_shader) -> void;
}
