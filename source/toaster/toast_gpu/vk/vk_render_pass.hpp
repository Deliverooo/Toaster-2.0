#pragma once

#include "toast_lib/core_basic.hpp"

#include "vk_pipeline.hpp"
#include "vk_render_attachment.hpp"
#include "vk_shader.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	struct RenderPassBeginInfo
	{
		RefPtr<VKPipeline> pipeline{nullptr};
		uint32             width{0u};
		uint32             height{0u};

		std::vector<VKRenderAttachment> attachments{};
	};
}
