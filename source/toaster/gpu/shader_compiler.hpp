#pragma once

#include <unordered_map>
#include <nvrhi/nvrhi.h>
#include <vulkan/vulkan.hpp>
#include "shader_common.hpp"

#include "io/filesystem.hpp"

namespace toaster::gpu::shader_compiler
{
	bool compileShaderSource(const io::filesystem::Path &p_shader_path, nvrhi::ShaderType p_shader_stage, std::vector<uint32> &p_out_binary);
}
