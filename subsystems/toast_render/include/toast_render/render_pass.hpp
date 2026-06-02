#pragma once

#include "descriptor_set_manager.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"

namespace toaster::render
{
	class TST_RENDER_API RenderPass
	{
	public:
		RenderPass(RenderContext &p_render_ctx, const gpu::PipelineHandle &p_pipeline);
		~RenderPass();

		template<gpu::GPUResource_c TResource>
		auto setInput(const String &p_name, const RefPtr<TResource> &p_resource) -> RenderPass &
		{
			m_descriptorSetManager->setDescriptor(p_name, p_resource);
			return *this;
		}

		// Only call when you have set all your required inputs :)
		auto bake() -> void;
		auto update(uint32 p_frame_index) -> void;

		auto               getPipeline() const -> const gpu::PipelineHandle &;
		[[nodiscard]] auto getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>;
		[[nodiscard]] auto getStartSetIndex() const -> uint32;
		[[nodiscard]] auto getEndSetIndex() const -> uint32;

	private:
		NonOwningPtr<RenderContext> m_renderCtx{nullptr};

		mutable gpu::PipelineHandle     m_pipeline{nullptr};
		OwningPtr<DescriptorSetManager> m_descriptorSetManager{nullptr};
	};

	TST_RENDER_DEFINE_HANDLE(RenderPass, RenderPass);
}
