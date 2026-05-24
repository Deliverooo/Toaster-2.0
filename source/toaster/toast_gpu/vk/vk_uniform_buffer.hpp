#pragma once

#include "../resource.hpp"
#include "../toast_gpu.hpp"

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	using UBOMappedData = void *;

	class TST_GPU_API VKUniformBuffer final : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(UniformBuffer)
	public:
		VKUniformBuffer(VKLogicalDevice *p_ctx, uint64 p_size);
		~VKUniformBuffer();

		[[nodiscard]] auto getBuffer() -> vk::Buffer &;
		[[nodiscard]] auto getBufferMemory() -> vk::DeviceMemory &;

		[[nodiscard]] auto getDescriptorInfo() const -> const vk::DescriptorBufferInfo &;

		auto setData(void *p_data, uint64 p_size, uint64 p_offset) -> void;

		auto mapMemory(uint64 p_size, uint64 p_offset) -> UBOMappedData;
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

	private:
		vk::Buffer               m_buffer{nullptr};
		vk::DeviceMemory         m_bufferMemory{nullptr};
		vk::DescriptorBufferInfo m_descriptorInfo{};
	};

	TST_GPU_DEFINE_HANDLE(VKUniformBuffer, UniformBuffer)

	using UBOMappedDataPFF = std::vector<void *>;

	// Uniform buffer, but it's per frame in flight
	// Ts is easier to use than allocating them manually, so it just makes things more manageable
	// Typically you wouldn't really use a single uniform buffer anyway, so use this instead.
	class TST_GPU_API VKUniformBufferPFF final : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(UniformBufferPFF)
	public:
		VKUniformBufferPFF(VKLogicalDevice *p_device, uint64 p_size, uint32 p_frames_in_flight);
		~VKUniformBufferPFF();

		[[nodiscard]] auto getBuffer(uint32 p_frame_index) -> vk::Buffer &;
		[[nodiscard]] auto getBufferMemory(uint32 p_frame_index) -> vk::DeviceMemory &;

		[[nodiscard]] auto getDescriptorInfo(uint32 p_frame_index) const -> const vk::DescriptorBufferInfo &;

		auto mapMemory(uint32 p_frame_index, uint64 p_size, uint64 p_offset = 0u) -> void *;
		auto unmapMemory(uint32 p_frame_index) -> void;

		auto mapAllMemory(uint64 p_size, uint64 p_offset = 0u) -> UBOMappedDataPFF;
		auto unmapAllMemory() -> void;

	private:
		std::vector<vk::Buffer>               m_uniformBuffers;
		std::vector<vk::DeviceMemory>         m_uniformBufferMemories;
		std::vector<vk::DescriptorBufferInfo> m_descriptorBufferInfos;
		uint32                                m_framesInFlightCount{0u};
	};

	TST_GPU_DEFINE_HANDLE(VKUniformBufferPFF, UniformBufferPFF)
}
