#pragma once

#include "vk_shader.hpp"
#include "toast_lib/ptr.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKComputePipeline
	{
	public:
		VKComputePipeline(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader);

		auto getContext() const -> VKGPUContext *;

		auto               getShader() const -> const RefPtr<VKShader> &;
		[[nodiscard]] auto getPipeline() -> vk::raii::Pipeline &;
		[[nodiscard]] auto getPipelineLayout() -> vk::raii::PipelineLayout &;

	private:
		VKGPUContext *m_ctx{nullptr};

		RefPtr<VKShader> m_shader{nullptr};

		vk::raii::Pipeline       m_pipeline{nullptr};
		vk::raii::PipelineLayout m_pipelineLayout{nullptr};
	};
}
