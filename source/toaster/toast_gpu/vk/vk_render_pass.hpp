#pragma once

#include "vk_pipeline.hpp"
#include "vk_uniform_buffer.hpp"
#include "vk_descriptor_set_manager.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKRenderPass
	{
	public:
		VKRenderPass(VKGPUContext *p_ctx, const RefPtr<VKPipeline> &p_pipeline);

		void setInput(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer);
		void setInput(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff);

		// Only call when you have set all your required inputs :)
		void bake();

		const std::vector<vk::raii::DescriptorSet> &getDescriptorSets(uint32 p_frame_index) const;

	private:
		VKGPUContext *m_ctx{nullptr};

		RefPtr<VKPipeline> m_pipeline{nullptr};

		UniquePtr<VKDescriptorSetManager> m_descriptorSetManager{nullptr};
	};
}
