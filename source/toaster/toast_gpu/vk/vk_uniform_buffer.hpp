#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"
#include "../resource.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKUniformBuffer final : public IGPUResource
	{
	public:
		VKUniformBuffer(VKGPUContext *p_ctx, uint64 p_size);
		[[nodiscard]] auto getContext() const -> VKGPUContext *;

		auto getBuffer() -> vk::raii::Buffer &;
		auto getBufferMemory() -> vk::raii::DeviceMemory &;

		[[nodiscard]] auto getDescriptorInfo() const -> const vk::DescriptorBufferInfo &;

		auto setData(void *p_data, uint64 p_size, uint64 p_offset) -> void;

		auto mapMemory(uint64 p_size, uint64 p_offset) -> void *;
		auto unmapMemory() -> void;

		[[nodiscard]] auto getResourceType() const -> EGPUResourceType override;

	private:
		VKGPUContext *m_ctx{nullptr};

		vk::raii::Buffer         m_buffer{nullptr};
		vk::raii::DeviceMemory   m_bufferMemory{nullptr};
		vk::DescriptorBufferInfo m_descriptorInfo{};
	};

	// Uniform buffer, but it's per frame in flight
	// Ts is easier to use than allocating them manually, so it just makes things more manageable
	// Typically you wouldn't really use a single uniform buffer anyway, so use this instead.
	class VKUniformBufferPFF final : public IGPUResource
	{
	public:
		VKUniformBufferPFF(VKGPUContext *p_ctx, uint64 p_size, uint32 p_frames_in_flight);

		auto getUBO(uint32 p_frame_index) -> RefPtr<VKUniformBuffer>;
		auto setUBO(uint32 p_frame_index, const RefPtr<VKUniformBuffer> &p_uniform_buffer) -> void;

		auto begin() -> std::vector<RefPtr<VKUniformBuffer> >::iterator;
		auto end() -> std::vector<RefPtr<VKUniformBuffer> >::iterator;

		[[nodiscard]] auto getResourceType() const -> EGPUResourceType override;

		auto mapMemory(uint64 p_size, uint64 p_offset) -> std::vector<void *>;
		auto unmapMemory() -> void;

	private:
		std::vector<RefPtr<VKUniformBuffer> > m_uniformBuffers;
		uint32                                m_framesInFlightCount{0u};
	};
}
