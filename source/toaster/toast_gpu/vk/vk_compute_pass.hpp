#pragma once

#include "vk_compute_pipeline.hpp"
#include "vk_uniform_buffer.hpp"
#include "vk_descriptor_set_manager.hpp"
#include "vk_texture.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;

	class TST_GPU_API VKComputePass
	{
	public:
		VKComputePass(VKLogicalDevice *p_device, const RefPtr<VKComputePipeline> &p_pipeline);
		auto getDevice() const -> VKLogicalDevice *;

		auto setInput(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer) -> void;
		auto setInput(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff) -> void;
		auto setInput(const String &p_name, const RefPtr<VKStorageBuffer> &p_storage_buffer) -> void;
		auto setInput(const String &p_name, const RefPtr<VKStorageBufferPFF> &p_storage_buffer_pff) -> void;
		auto setInput(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d) -> void;

		// Only call when you have set all your required inputs :)
		auto bake() -> void;
		auto update(uint32 p_frame_index) -> void;

		auto               getPipeline() const -> const RefPtr<VKComputePipeline> &;
		[[nodiscard]] auto getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>;
		[[nodiscard]] auto getStartSetIndex() const -> uint32;
		[[nodiscard]] auto getEndSetIndex() const -> uint32;

	private:
		VKLogicalDevice *m_device{nullptr};

		RefPtr<VKComputePipeline>         m_pipeline{nullptr};
		UniquePtr<VKDescriptorSetManager> m_descriptorSetManager{nullptr};
	};
}
