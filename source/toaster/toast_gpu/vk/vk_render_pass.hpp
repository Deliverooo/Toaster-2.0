#pragma once

#include "vk_pipeline.hpp"
#include "vk_uniform_buffer.hpp"
#include "vk_descriptor_set_manager.hpp"
#include "vk_texture.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKRenderPass
	{
	public:
		VKRenderPass(VKGPUContext *p_ctx, const RefPtr<VKPipeline> &p_pipeline);
		VKGPUContext *getContext() const;

		void setInput(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer);
		void setInput(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff);
		void setInput(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d);

		// Only call when you have set all your required inputs :)
		void bake();

		void update(uint32 p_frame_index);

		const RefPtr<VKPipeline> &                   getPipeline() const;
		[[nodiscard]] std::vector<vk::DescriptorSet> getDescriptorSets(uint32 p_frame_index) const;
		[[nodiscard]] uint32                         getStartSetIndex() const;
		[[nodiscard]] uint32                         getEndSetIndex() const;

	private:
		VKGPUContext *m_ctx{nullptr};

		RefPtr<VKPipeline> m_pipeline{nullptr};

		UniquePtr<VKDescriptorSetManager> m_descriptorSetManager{nullptr};
	};
}
