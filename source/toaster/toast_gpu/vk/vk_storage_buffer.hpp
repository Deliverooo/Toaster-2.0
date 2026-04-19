#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"
#include "../resource.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKStorageBuffer final : public IGPUResource
	{
	public:
		VKStorageBuffer(VKGPUContext *p_ctx, uint64 p_size);
		[[nodiscard]] auto getContext() const -> VKGPUContext *;

		auto getBuffer() -> vk::raii::Buffer &;
		auto getBufferMemory() -> vk::raii::DeviceMemory &;

		auto mapMemory(uint64 p_offset, uint64 p_size) -> void *;
		auto unmapMemory() -> void;

		auto resize(uint64 p_size) -> void;

		[[nodiscard]] auto getDescriptorInfo() const -> const vk::DescriptorBufferInfo &;
		[[nodiscard]] auto getResourceType() const -> EGPUResourceType override;

	private:
		VKGPUContext *m_ctx{nullptr};

		vk::raii::Buffer         m_buffer{nullptr};
		vk::raii::DeviceMemory   m_bufferMemory{nullptr};
		vk::DescriptorBufferInfo m_descriptorInfo{};
	};

	// Storage buffer, but it's per frame in flight
	// Ts is easier to use than allocating them manually, so it just makes things more manageable
	// Typically you wouldn't really use a single uniform buffer anyway, so use this instead.
	class VKStorageBufferPFF final : public IGPUResource
	{
	public:
		VKStorageBufferPFF(VKGPUContext *p_ctx, uint64 p_size, uint32 p_frames_in_flight);

		auto getSSBO(uint32 p_frame_index) -> RefPtr<VKStorageBuffer>;
		auto setSSBO(uint32 p_frame_index, const RefPtr<VKStorageBuffer> &p_storage_buffer) -> void;

		auto begin() -> std::vector<RefPtr<VKStorageBuffer> >::iterator;
		auto end() -> std::vector<RefPtr<VKStorageBuffer> >::iterator;

		auto resize(uint64 p_size) -> void;

		[[nodiscard]] auto getResourceType() const -> EGPUResourceType override;

	private:
		std::vector<RefPtr<VKStorageBuffer> > m_storageBuffers;
		uint32                                m_framesInFlightCount{0u};
	};
}
