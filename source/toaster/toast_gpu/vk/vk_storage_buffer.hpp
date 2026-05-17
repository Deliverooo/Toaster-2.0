#pragma once

#include "../resource.hpp"
#include "../toast_gpu.hpp"

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class TST_GPU_API VKStorageBuffer final : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(StorageBuffer)
	public:
		VKStorageBuffer(VKLogicalDevice *p_device, uint64 p_size);
		~VKStorageBuffer();

		auto getBuffer() -> vk::Buffer &;
		auto getBufferMemory() -> vk::DeviceMemory &;

		auto mapMemory(uint64 p_offset, uint64 p_size) -> void *;
		auto unmapMemory() -> void;

		template<typename TStorage>
		auto getStorage() -> TStorage
		{
			const void *mapped{mapMemory(0u, sizeof(TStorage))};
			TStorage    storage;
			std::memcpy(&storage, mapped, sizeof(TStorage));
			unmapMemory();
			return std::move(storage);
		}

		auto resize(uint64 p_size) -> void;

		[[nodiscard]] auto getDescriptorInfo() const -> const vk::DescriptorBufferInfo &;

	private:
		vk::Buffer               m_buffer{nullptr};
		vk::DeviceMemory         m_bufferMemory{nullptr};
		vk::DescriptorBufferInfo m_descriptorInfo{};
	};

	TST_GPU_DEFINE_HANDLE(VKStorageBuffer, StorageBuffer)

	// Storage buffer, but it's per frame in flight
	// Ts is easier to use than allocating them manually, so it just makes things more manageable
	// Typically you wouldn't really use a single uniform buffer anyway, so use this instead.
	class TST_GPU_API VKStorageBufferPFF final : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(StorageBufferPFF)
	public:
		VKStorageBufferPFF(VKLogicalDevice *p_device, uint64 p_size, uint32 p_frames_in_flight);
		~VKStorageBufferPFF();

		[[nodiscard]] auto getBuffer(uint32 p_frame_index) -> vk::Buffer &;
		[[nodiscard]] auto getBufferMemory(uint32 p_frame_index) -> vk::DeviceMemory &;

		[[nodiscard]] auto getDescriptorInfo(uint32 p_frame_index) const -> const vk::DescriptorBufferInfo &;

		auto mapMemory(uint32 p_frame_index, uint64 p_size, uint64 p_offset = 0u) -> void *;
		auto unmapMemory(uint32 p_frame_index) -> void;

		auto mapAllMemory(uint64 p_size, uint64 p_offset = 0u) -> std::vector<void *>;
		auto unmapAllMemory() -> void;

	private:
		std::vector<vk::Buffer>               m_storageBuffers;
		std::vector<vk::DeviceMemory>         m_storageBufferMemories;
		std::vector<vk::DescriptorBufferInfo> m_descriptorBufferInfos;
		uint32                                m_framesInFlightCount{0u};
	};

	TST_GPU_DEFINE_HANDLE(VKStorageBufferPFF, StorageBufferPFF)
}
