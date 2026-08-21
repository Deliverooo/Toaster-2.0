#pragma once

#include "descriptor_heap.hpp"
#include "gpu_common.hpp"
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
		TST_REGISTER_DEPENDENCY(LogicalDevice, device)
		TST_REGISTER_DEPENDENCY(Allocator, allocator)
	public:
		using DestroyCallback = void(*)(void *, BufferHandle);
		using PoolType        = Pool<BufferTag, BufferData>;

		BufferManager(LogicalDevice &p_device, Allocator &p_allocator, void *p_user_data = nullptr, const DestroyCallback &p_destroy_callback = nullptr);
		~BufferManager();

		auto setUserData(void *p_user_data) -> void { m_userData = p_user_data; }
		auto setDestroyCallback(const DestroyCallback &p_destroy_callback) -> void { m_destroyCallback = p_destroy_callback; }

		[[nodiscard]] auto createBuffer(const BufferDesc &p_desc) -> BufferHandle;
		auto               createSharedBuffer(const BufferDesc &p_desc) -> SharedHandle<BufferTag, BufferData>;
		auto               destroyBuffer(BufferHandle p_handle) -> void { m_pool.destroy(p_handle); } // Secretly defers it...

		// Used for external callers to fall back to the default destruction logic, so DON'T use outside of callbacks
		auto destroyData(BufferData *p_data) -> void;

		auto getData(BufferHandle p_handle) const -> const BufferData * { return m_pool.getData(p_handle); }
		auto getData(BufferHandle p_handle) -> BufferData * { return m_pool.getData(p_handle); }

		auto getBufferBuffer(BufferHandle p_handle) -> vk::Buffer { return getData(p_handle)->buffer; }
		auto getBufferAllocation(BufferHandle p_handle) -> VmaAllocation { return getData(p_handle)->allocation; }
		auto getBufferSize(BufferHandle p_handle) -> vk::DeviceSize { return getData(p_handle)->size; }
		auto getBufferDeviceAddress(BufferHandle p_handle) -> vk::DeviceAddress { return getData(p_handle)->address; }
		auto getBufferUsageFlags(BufferHandle p_handle) -> vk::BufferUsageFlags { return getData(p_handle)->usageFlags; }
		auto getBufferMappedPtr(BufferHandle p_handle) -> void * { return getData(p_handle)->mapped; }

		auto uploadDirect(BufferHandle p_handle, const void *p_data, uint64 p_size, uint64 p_offset = 0u) -> void; // Uploads data to a host-visible buffer

		auto getPoolData(uint32 p_id) -> BufferData * { return &m_pool._data[p_id]; }
		auto getPoolData(uint32 p_id) const -> const BufferData * { return &m_pool._data[p_id]; }

	private:
		PoolType m_pool;

		void *          m_userData{nullptr};
		DestroyCallback m_destroyCallback{nullptr}; // Yes... There are two destroy callbacks going on here, but it allows for per frame deferred deletion
	};

	using SharedBuffer = SharedHandle<BufferTag, BufferData>;
}
