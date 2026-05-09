#pragma once

#include "vk_pipeline.hpp"
#include "vk_uniform_buffer.hpp"
#include "vk_descriptor_set_manager.hpp"
#include "vk_texture.hpp"

namespace toaster::gpu
{
	class TST_GPU_API VKRenderPass
	{
		TST_GPU_OBJECT
	public:
		VKRenderPass(VKLogicalDevice *p_device, const RefPtr<VKPipeline> &p_pipeline);

		auto setInput(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer) -> void;
		auto setInput(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff) -> void;
		auto setInput(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d) -> void;
		auto setInput(const String &p_name, const RefPtr<VKImage2D> &p_image_2d) -> void;

		// Only call when you have set all your required inputs :)
		auto bake() -> void;
		auto update(uint32 p_frame_index) -> void;

		auto               getPipeline() const -> const RefPtr<VKPipeline> &;
		[[nodiscard]] auto getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>;
		[[nodiscard]] auto getStartSetIndex() const -> uint32;
		[[nodiscard]] auto getEndSetIndex() const -> uint32;

	private:
		RefPtr<VKPipeline>                m_pipeline{nullptr};
		UniquePtr<VKDescriptorSetManager> m_descriptorSetManager{nullptr};
	};
}
