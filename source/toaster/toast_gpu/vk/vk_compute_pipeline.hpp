#pragma once

#include "vk_shader.hpp"
#include "toast_lib/ptr.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;

	class VKComputePipeline
	{
	public:
		VKComputePipeline(VKLogicalDevice *p_device, const RefPtr<VKShader> &p_shader);
		[[nodiscard]] auto getDevice() const -> VKLogicalDevice *;

		auto               getShader() const -> const RefPtr<VKShader> &;
		[[nodiscard]] auto getPipeline() -> vk::raii::Pipeline &;
		[[nodiscard]] auto getPipelineLayout() -> vk::raii::PipelineLayout &;

	private:
		VKLogicalDevice *m_device{nullptr};

		RefPtr<VKShader> m_shader{nullptr};

		vk::raii::Pipeline       m_pipeline{nullptr};
		vk::raii::PipelineLayout m_pipelineLayout{nullptr};
	};
}
