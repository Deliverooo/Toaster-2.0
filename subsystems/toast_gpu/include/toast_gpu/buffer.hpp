#pragma once

#include "descriptor_heap.hpp"
#include "toast_lib/pool.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API BufferData
	{
		vk::Buffer        buffer{nullptr};
		VmaAllocation     allocation{nullptr};
		vk::DeviceSize    size{0u};
		vk::DeviceAddress address{0u};

		void *mapped{nullptr}; // nullptr if the buffer is not host visible / coherent

		vk::BufferUsageFlags usageFlags{};
		EMemoryProperties    memoryProperties{EMemoryProperties::eDeviceLocal};
	};

	TST_DECLARE_HANDLE(Buffer);
	TST_DECLARE_REF(Buffer);

	struct TST_GPU_API BufferDesc
	{
		static BufferDesc staging(vk::DeviceSize p_size)
		{
			return BufferDesc{p_size, vk::BufferUsageFlagBits::eTransferSrc, EMemoryProperties::eHostVisibleCoherent};
		}

		vk::DeviceSize       size{0u};
		vk::BufferUsageFlags usageFlags{};
		EMemoryProperties    memoryProperties{EMemoryProperties::eDeviceLocal};
	};
}

namespace toaster
{
	template<>
	class ResourceManager<gpu::BufferTag, gpu::BufferData>
	{
	public:
		using HandleType = gpu::BufferHandle;
		using DataType   = gpu::BufferData;
		using RefType    = Ref<gpu::BufferTag, DataType>;
		using Deleter    = void(*)(void *, DataType *);

		ResourceManager() = default;

		ResourceManager(Deleter p_deleter, void *p_deleter_user_data, gpu::ResourceDescriptorHeap *p_resource_heap) : m_deleter(p_deleter),
																													  m_deleterUserData(p_deleter_user_data),
																													  m_resourceHeap(p_resource_heap)
		{
			TST_ASSERT(m_deleter);
		}

		auto setDeleter(Deleter p_deleter) { m_deleter = p_deleter; }
		auto setDeleterUserData(void *p_deleter_user_data) { m_deleterUserData = p_deleter_user_data; }
		auto setResourceHeap(gpu::ResourceDescriptorHeap *p_resource_heap) { m_resourceHeap = p_resource_heap; }

		template<typename... TArgs>
		[[nodiscard]] auto create(TArgs &&... p_args) -> RefType;

		auto acquire(HandleType p_handle) -> void { m_pool.incRef(p_handle); }

		auto release(HandleType p_handle) -> void
		{
			if (DataType *data{m_pool.decRef(p_handle)})
				m_deleter(m_deleterUserData, data);
		}

		auto isValid(HandleType p_handle) const -> bool { return m_pool.isValid(p_handle); }
		auto getData(HandleType p_handle) const -> const DataType * { return m_pool.getData(p_handle); }
		auto getData(HandleType p_handle) -> DataType * { return m_pool.getData(p_handle); }

		template<typename TFunc>
		auto forEachAlive(TFunc &&p_func) -> void
		{
			m_pool.forEachAlive(std::forward<TFunc>(p_func));
		}

	private:
		Pool<gpu::BufferTag, DataType> m_pool;

		Deleter m_deleter{nullptr};
		void *  m_deleterUserData{nullptr};

		gpu::ResourceDescriptorHeap *m_resourceHeap{nullptr}; // To get the heap ids
	};

	template<>
	class Ref<gpu::BufferTag, gpu::BufferData>
	{
	public:
		using ManagerType = ResourceManager<gpu::BufferTag, gpu::BufferData>;
		using HandleType  = Handle<gpu::BufferTag>;

		Ref() noexcept = default;

		Ref(ManagerType *p_manager, HandleType p_handle) noexcept : m_manager(p_manager), m_handle(p_handle)
		{
		}

		~Ref() { reset(); }

		Ref(const Ref &p_other) noexcept : m_manager(p_other.m_manager), m_handle(p_other.m_handle)
		{
			if (m_manager && m_handle.valid())
				m_manager->acquire(m_handle);
		}

		Ref(Ref &&p_other) noexcept : m_manager(p_other.m_manager), m_handle(p_other.m_handle)
		{
			p_other.m_manager = nullptr;
			p_other.m_handle  = {};
		}

		Ref &operator=(const Ref &p_other) noexcept
		{
			if (this != &p_other)
			{
				reset();

				m_manager = p_other.m_manager;
				m_handle  = p_other.m_handle;

				if (m_manager && m_handle.valid())
					m_manager->acquire(m_handle);
			}
			return *this;
		}

		Ref &operator=(Ref &&p_other) noexcept
		{
			if (this != &p_other)
			{
				reset();

				m_manager = p_other.m_manager;
				m_handle  = p_other.m_handle;

				p_other.m_manager = nullptr;
				p_other.m_handle  = {};
			}
			return *this;
		}

		void reset() noexcept
		{
			if (m_manager && m_handle.valid())
				m_manager->release(m_handle);

			m_manager = nullptr;
			m_handle  = {};
		}

		[[nodiscard]] HandleType get() const noexcept { return m_handle; }

		[[nodiscard]] bool valid() const noexcept { return m_manager && m_handle.valid() && m_manager->isValid(m_handle); }

		[[nodiscard]] auto operator->() const -> auto { return m_manager->getData(m_handle); }
		[[nodiscard]] auto operator->() -> auto { return m_manager->getData(m_handle); }

		[[nodiscard]] auto manager() -> ManagerType * { return m_manager; }
		[[nodiscard]] auto manager() const -> const ManagerType * { return m_manager; }

		explicit operator bool() const noexcept { return valid(); }

		HandleType operator*() const noexcept { return m_handle; }

	private:
		ManagerType *m_manager{nullptr};
		HandleType   m_handle{};
	};

	// This has to be after the buffer template specialisation
	template<typename... TArgs>
	auto ResourceManager<gpu::BufferTag, gpu::BufferData>::create(TArgs &&... p_args) -> RefType
	{
		HandleType handle{m_pool.emplace(std::forward<TArgs>(p_args)...)};
		m_pool.incRef(handle);
		return RefType{this, handle};
	}
}
