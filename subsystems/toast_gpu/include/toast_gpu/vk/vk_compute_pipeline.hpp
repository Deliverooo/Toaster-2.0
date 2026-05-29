#pragma once

#include "vk_shader.hpp"
#include "toast_lib/ptr.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;

	class TST_GPU_API VKComputePipeline
	{
		TST_GPU_OBJECT
	public:
		VKComputePipeline(VKLogicalDevice *p_device, const RefPtr<VKShader> &p_shader);

		auto               getShader() const -> const RefPtr<VKShader> &;
		[[nodiscard]] auto getPipeline() -> vk::raii::Pipeline &;
		[[nodiscard]] auto getPipelineLayout() -> vk::raii::PipelineLayout &;

		operator vk::Pipeline() const;

	private:
		RefPtr<VKShader> m_shader{nullptr};

		vk::raii::Pipeline       m_pipeline{nullptr};
		vk::raii::PipelineLayout m_pipelineLayout{nullptr};
	};

	TST_GPU_DEFINE_HANDLE(VKComputePipeline, ComputePipeline)
}
