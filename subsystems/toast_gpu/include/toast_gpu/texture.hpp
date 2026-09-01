#pragma once

#include "descriptor_heap.hpp"
#include "toast_lib/pool.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API TextureData
	{
		vk::Extent3D extent;

		vk::Image     image{nullptr};
		vk::ImageView imageView{nullptr}; // Completely optional unless you are creating a render target
		VmaAllocation allocation{nullptr};

		vk::ImageUsageFlags usageFlags{};
		uint32              layerCount{1u};
		uint32              mipLevels{1u};

		vk::Format      format{vk::Format::eUndefined};
		vk::ImageLayout layout{vk::ImageLayout::eUndefined};
		vk::ImageType   type{vk::ImageType::e2D};

		DescriptorSlot shaderReadHeapID{invalidImageDescriptorSlot};
		DescriptorSlot storageHeapID{invalidImageDescriptorSlot};
	};

	TST_DECLARE_HANDLE(Texture);
	TST_DECLARE_REF(Texture);

	struct TST_GPU_API TextureDesc
	{
		vk::Extent3D extent;

		vk::Image existingImage{nullptr}; // Use for swapchain images

		vk::ImageUsageFlags usageFlags{}; // Determines if the device should create image views for render attachments
		uint32              layerCount{1u};
		uint32              mipLevels{1u};

		vk::Format    format{vk::Format::eUndefined};
		vk::ImageType type{vk::ImageType::e2D};

		bool createDescriptors{false}; // Creates the heap id's depending on the image's usage flags
	};
}

namespace toaster
{
	template<>
	class ResourceManager<gpu::TextureTag, gpu::TextureData>
	{
	public:
		using HandleType = gpu::TextureHandle;
		using RefType    = Ref<gpu::TextureTag, gpu::TextureData>;
		using Deleter    = void(*)(void *, gpu::TextureData *);

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
			if (gpu::TextureData *data{m_pool.decRef(p_handle)})
				m_deleter(m_deleterUserData, data);
		}

		auto isValid(HandleType p_handle) const -> bool { return m_pool.isValid(p_handle); }
		auto getData(HandleType p_handle) const -> const gpu::TextureData * { return m_pool.getData(p_handle); }
		auto getData(HandleType p_handle) -> gpu::TextureData * { return m_pool.getData(p_handle); }

		// This is actually different from TextureData::shaderReadHeapID.
		// Because TextureData::shaderReadHeapID is relative to the start of the image descriptor block and not the resource heap as a whole
		auto getTextureShaderReadHeapSlot(HandleType p_handle) const -> uint32 { return m_resourceHeap->getImageAbsoluteHeapSlot(getData(p_handle)->shaderReadHeapID); }
		auto getTextureStorageHeapSlot(HandleType p_handle) const -> uint32 { return m_resourceHeap->getImageAbsoluteHeapSlot(getData(p_handle)->storageHeapID); }

		template<typename TFunc>
		auto forEachAlive(TFunc &&p_func) -> void
		{
			m_pool.forEachAlive(std::forward<TFunc>(p_func));
		}

	private:
		Pool<gpu::TextureTag, gpu::TextureData> m_pool;

		Deleter m_deleter{nullptr};
		void *  m_deleterUserData{nullptr};

		gpu::ResourceDescriptorHeap *m_resourceHeap{nullptr}; // To get the heap ids
	};

	template<>
	class Ref<gpu::TextureTag, gpu::TextureData>
	{
	public:
		using ManagerType = ResourceManager<gpu::TextureTag, gpu::TextureData>;
		using HandleType  = Handle<gpu::TextureTag>;

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

		[[nodiscard]] auto shaderReadSlot() const -> uint32 { return m_manager->getTextureShaderReadHeapSlot(m_handle); }
		[[nodiscard]] auto storageSlot() const -> uint32 { return m_manager->getTextureStorageHeapSlot(m_handle); }

		explicit operator bool() const noexcept { return valid(); }

		HandleType operator*() const noexcept { return m_handle; }

	private:
		ManagerType *m_manager{nullptr};
		HandleType   m_handle{};
	};

	// This has to be after the texture template specialisation
	template<typename... TArgs>
	auto ResourceManager<gpu::TextureTag, gpu::TextureData>::create(TArgs &&... p_args) -> RefType
	{
		HandleType handle{m_pool.emplace(std::forward<TArgs>(p_args)...)};
		m_pool.incRef(handle);
		return RefType{this, handle};
	}
}
