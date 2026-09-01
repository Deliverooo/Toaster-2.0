#pragma once

#include "gpu_enums.hpp"
#include "toast_lib/pool.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API ShaderData
	{
		vk::ShaderEXT shader{nullptr};

		EShaderStageBits  stage{EShaderStageBits::eVertex};
		EShaderStageFlags nextStage{0u}; // The set of valid stages that could be next
	};

	TST_DECLARE_HANDLE(Shader);
	TST_DECLARE_REF(Shader);

	struct TST_GPU_API ShaderDesc
	{
		const void *code{nullptr};
		uint64      codeSize{0u};

		EShaderStageBits  stage{EShaderStageBits::eVertex};
		EShaderStageFlags nextStage{0u}; // The set of valid stages that could be next
	};
}
