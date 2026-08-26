#pragma once

#include "allocator.hpp"
#include "deletion_queue.hpp"
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
	using SharedBuffer = SharedHandle<BufferTag, BufferData>;

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

		void *            mapped{nullptr}; // I guess you can have mapped image memory...
		EMemoryProperties memoryProperties{EMemoryProperties::eDeviceLocal};

		DescriptorSlot shaderReadHeapID{invalidImageDescriptorSlot};
		DescriptorSlot storageHeapID{invalidImageDescriptorSlot};
	};

	TST_DECLARE_HANDLE(Texture);
	using SharedTexture = SharedHandle<TextureTag, TextureData>;

	struct TST_GPU_API TextureDesc
	{
		vk::Extent3D extent;

		vk::ImageUsageFlags usageFlags{};
		uint32              layerCount{1u};
		uint32              mipLevels{1u};

		vk::Format    format{vk::Format::eUndefined};
		vk::ImageType type{vk::ImageType::e2D};

		EMemoryProperties memoryProperties{EMemoryProperties::eDeviceLocal};

		bool createDescriptors{false}; // Creates the heap id's depending on the image's usage flags
	};

	struct TST_GPU_API DeviceDesc
	{
		bool  enableDebugInfo{true}; // Translates directly to whether validation layers are enabled.
		bool  usingSwapchain{true};  // If true, the GPUContext will use the required instance and device extensions
		uint8 numDeletionQueues{3u}; // Set this to your max frames in flight. (Should be 3 already)

		// For the resource and sampler heaps, these define the maximum number of descriptors that can allocated of the specified type
		uint32 maxBufferDescriptors{32u};
		uint32 maxImageDescriptors{32u};
		uint32 maxSamplerDescriptors{8u}; // You probably don't need that many samplers (Don't have one per image!)
	};

	class CommandList;

	// Represents the absolute minimum required to set up a Vulkan 'context' and create other objects. And a deletion queue
	class TST_GPU_API Device
	{
	public:
		Device(DeviceDesc p_desc);
		~Device();

		Device(const Device &)            = delete;
		Device(Device &&)                 = delete;
		Device &operator=(const Device &) = delete;
		Device &operator=(Device &&)      = delete;

		auto getInstance() -> Instance & { return *m_instance; }
		auto getInstance() const -> const Instance & { return *m_instance; }

		auto getPhysicalDevice() -> PhysicalDevice & { return *m_physicalDevice; }
		auto getPhysicalDevice() const -> const PhysicalDevice & { return *m_physicalDevice; }

		auto getDevice() -> LogicalDevice & { return *m_device; }
		auto getDevice() const -> const LogicalDevice & { return *m_device; }

		auto getAllocator() -> Allocator & { return *m_allocator; }
		auto getAllocator() const -> const Allocator & { return *m_allocator; }

		#pragma region deletion queue

		// Submits an arbitrary invokable to the current deletion queue
		template<typename TFunc> requires std::is_invocable_v<TFunc> && std::is_nothrow_invocable_v<TFunc>
		auto submitDeletion(TFunc &&p_func) -> void
		{
			m_deletionQueue->submit(std::forward<TFunc>(p_func));
		}

		// Flushes the current deletion queue and swaps it
		auto performGarbageCollection(uint32 p_queue_index) -> void; // For the queue index, pass the current frame index...

		// Flushes the entire deletion queue and clears all pending deletions
		auto flushDeletionQueue() -> void;

		#pragma endregion

		#pragma region descriptors

		auto getResourceHeap() -> ResourceDescriptorHeap & { return *m_resourceHeap; }
		auto getResourceHeap() const -> const ResourceDescriptorHeap & { return *m_resourceHeap; }

		auto getSamplerHeap() -> SamplerDescriptorHeap & { return *m_samplerHeap; }
		auto getSamplerHeap() const -> const SamplerDescriptorHeap & { return *m_samplerHeap; }

		// So far, when creating textures and buffers, all descriptor heap writes have been queued. When the time is appropriate, call this to flush and write them
		auto updateResourceDescriptorWrites() -> void;
		auto updateSamplerDescriptorWrites() -> void;

		#pragma endregion

		#pragma region buffers

		[[nodiscard]] auto createBuffer(const BufferDesc &p_desc) -> BufferHandle;
		[[nodiscard]] auto createSharedBuffer(const BufferDesc &p_desc) -> SharedBuffer { return {createBuffer(p_desc), &m_bufferPool}; }
		auto               destroyBuffer(BufferHandle p_handle) -> void { m_bufferPool.destroy(p_handle); } // Invokes pool callback to defer deletion

		auto getBufferData(BufferHandle p_handle) const -> const BufferData * { return m_bufferPool.getData(p_handle); }
		auto getBufferData(BufferHandle p_handle) -> BufferData * { return m_bufferPool.getData(p_handle); }

		auto uploadBufferData(BufferHandle p_handle, const void *p_data, uint64 p_size, uint64 p_offset = 0u) -> void; // Uploads data to a host-visible buffer

		#pragma endregion

		#pragma region textures

		[[nodiscard]] auto createTexture(const TextureDesc &p_desc) -> TextureHandle;
		[[nodiscard]] auto createSharedTexture(const TextureDesc &p_desc) -> SharedTexture { return {createTexture(p_desc), &m_texturePool}; }
		auto               destroyTexture(TextureHandle p_handle) -> void { m_texturePool.destroy(p_handle); } // Invokes pool callback to defer deletion

		auto getTextureData(TextureHandle p_handle) const -> const TextureData * { return m_texturePool.getData(p_handle); }
		auto getTextureData(TextureHandle p_handle) -> TextureData * { return m_texturePool.getData(p_handle); }

		// This is actually different from TextureData::shaderReadHeapID.
		// Because TextureData::shaderReadHeapID is relative to the start of the image descriptor block and not the resource heap as a whole
		auto getTextureShaderReadHeapSlot(TextureHandle p_handle) const -> uint32;
		auto getTextureStorageHeapSlot(TextureHandle p_handle) const -> uint32;

		#pragma endregion

		auto createCommandList() -> CommandList;
		auto createCommandLists(/*EQueueType p_queue_type,*/ uint32 p_count) -> std::vector<CommandList>;

		auto executeCommandLists(const std::initializer_list<const CommandList *> &          p_command_lists,
								 const std::initializer_list<const vk::SemaphoreSubmitInfo> &p_wait_semaphores   = {},
								 const std::initializer_list<const vk::SemaphoreSubmitInfo> &p_signal_semaphores = {}, vk::Fence p_signal_fence = nullptr) const -> void;

		auto createTimelineSemaphore(uint64 p_initial_value = 0u) const -> vk::Semaphore;
		auto waitForTimelineSemaphores(const InitialiserList<const vk::Semaphore> &p_semaphores, const InitialiserList<const uint64> &p_target_values) const -> void;

		auto getBufferAddress(vk::Buffer p_buffer) const -> vk::DeviceAddress { return m_device->getDevice().getBufferAddress({p_buffer}); }

	private:
		Pool<BufferTag, BufferData>   m_bufferPool;
		Pool<TextureTag, TextureData> m_texturePool;

		UniquePtr<Instance>       m_instance{nullptr};
		UniquePtr<PhysicalDevice> m_physicalDevice{nullptr};
		UniquePtr<LogicalDevice>  m_device{nullptr};

		UniquePtr<Allocator>     m_allocator{nullptr};
		UniquePtr<DeletionQueue> m_deletionQueue;

		UniquePtr<ResourceDescriptorHeap> m_resourceHeap{nullptr};
		UniquePtr<SamplerDescriptorHeap>  m_samplerHeap{nullptr};

		friend class CommandList;
	};
}
