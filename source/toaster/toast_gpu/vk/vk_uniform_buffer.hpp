#pragma once

#include "../resource.hpp"
#include "../toast_gpu.hpp"

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class TST_GPU_API VKUniformBuffer final : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(UniformBuffer)
	public:
		VKUniformBuffer(VKLogicalDevice *p_ctx, uint64 p_size);

		[[nodiscard]] auto getBuffer() -> vk::raii::Buffer &;
		[[nodiscard]] auto getBufferMemory() -> vk::raii::DeviceMemory &;

		[[nodiscard]] auto getDescriptorInfo() const -> const vk::DescriptorBufferInfo &;

		auto setData(void *p_data, uint64 p_size, uint64 p_offset) -> void;

		auto mapMemory(uint64 p_size, uint64 p_offset) -> void *;
		auto unmapMemory() -> void;

	private:
		vk::raii::Buffer         m_buffer{nullptr};
		vk::raii::DeviceMemory   m_bufferMemory{nullptr};
		vk::DescriptorBufferInfo m_descriptorInfo{};
	};
	TST_GPU_DEFINE_HANDLE(VKUniformBuffer, UniformBuffer)


	// Uniform buffer, but it's per frame in flight
	// Ts is easier to use than allocating them manually, so it just makes things more manageable
	// Typically you wouldn't really use a single uniform buffer anyway, so use this instead.
	class TST_GPU_API VKUniformBufferPFF final : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(UniformBufferPFF)
	public:
		VKUniformBufferPFF(VKLogicalDevice *p_device, uint64 p_size, uint32 p_frames_in_flight);
		VKUniformBufferPFF(const VKUniformBufferPFF &p_other) = delete;
		VKUniformBufferPFF(VKUniformBufferPFF &&p_other)      = delete;
		auto operator=(VKUniformBufferPFF &&p_other) noexcept -> VKUniformBufferPFF &;

		[[nodiscard]] auto getBuffer(uint32 p_frame_index) -> vk::raii::Buffer &;
		[[nodiscard]] auto getBufferMemory(uint32 p_frame_index) -> vk::raii::DeviceMemory &;

		[[nodiscard]] auto getDescriptorInfo(uint32 p_frame_index) const -> const vk::DescriptorBufferInfo &;

		auto mapMemory(uint32 p_frame_index, uint64 p_size, uint64 p_offset = 0u) -> void *;
		auto unmapMemory(uint32 p_frame_index) -> void;

		auto mapAllMemory(uint64 p_size, uint64 p_offset = 0u) -> std::vector<void *>;
		auto unmapAllMemory() -> void;

	private:
		std::vector<vk::raii::Buffer>         m_uniformBuffers;
		std::vector<vk::raii::DeviceMemory>   m_uniformBufferMemories;
		std::vector<vk::DescriptorBufferInfo> m_descriptorBufferInfos;
		uint32                                m_framesInFlightCount{0u};
	};

	TST_GPU_DEFINE_HANDLE(VKUniformBufferPFF, UniformBufferPFF)
}
