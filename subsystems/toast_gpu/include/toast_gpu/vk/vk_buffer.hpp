#pragma once

#include "toast_gpu/toast_gpu.hpp"

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API BufferSpecInfo
	{
		vk::BufferUsageFlags2 usageFlags{};
		vk::QueueFlags        queueAccessFlags{vk::QueueFlagBits::eGraphics}; // Also determines if the sharing mode is exclusive or not
		bool32                deviceLocal{false};
	};

	class TST_GPU_API VKBuffer
	{
		TST_GPU_OBJECT
	public:
		VKBuffer() = default;
		auto operator=(VKBuffer &&p_other) noexcept -> VKBuffer &;
		VKBuffer(VKGPUContext& p_gpu_ctx, vk::DeviceSize p_size, const BufferSpecInfo &p_spec_info);
		~VKBuffer();

		[[nodiscard]] auto getSpecInfo() const -> const BufferSpecInfo &;
		[[nodiscard]] auto getSize() const -> vk::DeviceSize;
		[[nodiscard]] auto getBuffer() const -> vk::Buffer;
		[[nodiscard]] auto getBufferMemory() const -> vk::DeviceMemory;

		[[nodiscard]] auto getDeviceAddress() const -> vk::DeviceAddress;
		[[nodiscard]] auto getDeviceAddressRange() const -> vk::DeviceAddressRangeKHR;

		auto mapMemory(uint64 p_size, uint64 p_offset = 0u) -> void *;
		auto unmapMemory() -> void;

		// Only usable if not device local!!
		template<typename Type>
		auto setData(const Type &p_data)
		{
			TST_PERMA_ASSERT(!m_specInfo.deviceLocal);
			void *mapped{mapMemory(sizeof(Type))};
			std::memcpy(mapped, &p_data, sizeof(Type));
			unmapMemory();
		}

		auto setData(const void *p_data, uint64 p_size) -> void;

		auto copyFromBuffer(VKBuffer& p_other) -> void;

	private:
		BufferSpecInfo         m_specInfo{};
		vk::DeviceSize         m_size{0u};
		vk::raii::Buffer       m_buffer{nullptr};
		vk::raii::DeviceMemory m_bufferMemory{nullptr};
	};

	TST_GPU_DEFINE_HANDLE(VKBuffer, Buffer);
}
