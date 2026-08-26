#pragma once

#include "gpu_common.hpp"
#include "device.hpp"
#include "toast_lib/pool.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API BufferData
	{
		vk::Buffer           buffer{nullptr};
		VmaAllocation        allocation{nullptr};
		vk::DeviceSize       size{0u};
		vk::DeviceAddress    address{0u};
		vk::BufferUsageFlags usageFlags{};
		EMemoryProperties    memoryProperties{EMemoryProperties::eDeviceLocal};

		void *mapped{nullptr}; // nullptr if the buffer is not host visible / coherent
	};

	TST_DECLARE_HANDLE(Buffer);

	struct TST_GPU_API BufferDesc
	{
		vk::DeviceSize       size{0u};
		vk::BufferUsageFlags usageFlags{};
		EMemoryProperties    memoryProperties{EMemoryProperties::eDeviceLocal};
	};

	class TST_GPU_API BufferManager
	{
		TST_REGISTER_DEPENDENCY(Device, Device, gpuCtx)
	public:
		using PoolType = Pool<BufferTag, BufferData>;

		BufferManager(Device &p_gpu_ctx);
		~BufferManager();

		BufferManager(const BufferManager &)            = delete;
		BufferManager(BufferManager &&)                 = delete;
		BufferManager &operator=(const BufferManager &) = delete;
		BufferManager &operator=(BufferManager &&)      = delete;

		[[nodiscard]] auto createBuffer(const BufferDesc &p_desc) -> BufferHandle;
		auto               createSharedBuffer(const BufferDesc &p_desc) -> SharedHandle<BufferTag, BufferData>;
		auto               destroyBuffer(BufferHandle p_handle) -> void { m_pool.destroy(p_handle); } // Secretly defers it...

		auto getData(BufferHandle p_handle) const -> const BufferData * { return m_pool.getData(p_handle); }
		auto getData(BufferHandle p_handle) -> BufferData * { return m_pool.getData(p_handle); }

		auto getBufferBuffer(BufferHandle p_handle) -> vk::Buffer { return getData(p_handle)->buffer; }
		auto getBufferAllocation(BufferHandle p_handle) -> VmaAllocation { return getData(p_handle)->allocation; }
		auto getBufferSize(BufferHandle p_handle) -> vk::DeviceSize { return getData(p_handle)->size; }
		auto getBufferDeviceAddress(BufferHandle p_handle) -> vk::DeviceAddress { return getData(p_handle)->address; }
		auto getBufferUsageFlags(BufferHandle p_handle) -> vk::BufferUsageFlags { return getData(p_handle)->usageFlags; }
		auto getBufferMappedPtr(BufferHandle p_handle) -> void * { return getData(p_handle)->mapped; }

		auto uploadDirect(BufferHandle p_handle, const void *p_data, uint64 p_size, uint64 p_offset = 0u) -> void; // Uploads data to a host-visible buffer
		auto copyBuffer(BufferHandle p_src_handle, BufferHandle p_dst_handle, vk::CommandBuffer p_cmd, uint64 p_size, uint64 p_src_offset = 0u,
						uint64       p_dst_offset                                                                                         = 0u) -> void;

		auto getPoolData(uint32 p_id) -> BufferData * { return &m_pool._data[p_id]; }
		auto getPoolData(uint32 p_id) const -> const BufferData * { return &m_pool._data[p_id]; }

	private:
		PoolType m_pool;

		const char*datathing{"Orbo Stetson!!"};
	};

	using SharedBuffer = SharedHandle<BufferTag, BufferData>;
}
