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
		TST_GPU_OBJECT
	public:
		VKComputePass(VKLogicalDevice *p_device, const ComputePipelineHandle &p_pipeline);
		~VKComputePass();

		template<GPUResource_c TResource>
		auto setInput(const String &p_name, const RefPtr<TResource> &p_resource) -> VKComputePass &
		{
			m_descriptorSetManager->setDescriptor(p_name, p_resource);
			return *this;
		}

		// Only call when you have set all your required inputs :)
		auto bake() -> void;
		auto update(uint32 p_frame_index) -> void;

		auto               getPipeline() const -> const ComputePipelineHandle &;
		[[nodiscard]] auto getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>;
		[[nodiscard]] auto getStartSetIndex() const -> uint32;
		[[nodiscard]] auto getEndSetIndex() const -> uint32;

	private:
		ComputePipelineHandle             m_pipeline{nullptr};
		OwningPtr<VKDescriptorSetManager> m_descriptorSetManager{nullptr};
	};

	TST_GPU_DEFINE_HANDLE(VKComputePass, ComputePass)
}
