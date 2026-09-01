#pragma once

#include "allocator.hpp"
#include "deletion_queue.hpp"
#include "descriptor_heap.hpp"

#include "toast_lib/pool.hpp"
#include "gpu_enums.hpp"

namespace toaster::gpu
{
	class Device;

	#pragma region buffer
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
	#pragma endregion

	#pragma region texture
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
	#pragma endregion

	#pragma region sampler
	enum class EFilter : uint8
	{
		eNearest, eLinear
	};

	enum class ESamplerMipmapMode : uint8
	{
		eNearest, eLinear
	};

	enum class ESamplerAddressMode :uint8
	{
		eRepeat,
		eMirroredRepeat,
		eClampToEdge,
		eClampToBorder
	};

	struct TST_GPU_API SamplerData
	{
		EFilter             minFilter{EFilter::eLinear};
		EFilter             magFilter{EFilter::eLinear};
		ESamplerMipmapMode  mipmapMode{ESamplerMipmapMode::eLinear};
		ESamplerAddressMode addressModeU{ESamplerAddressMode::eRepeat};
		ESamplerAddressMode addressModeV{ESamplerAddressMode::eRepeat};
		ESamplerAddressMode addressModeW{ESamplerAddressMode::eRepeat};

		DescriptorSlot heapID{invalidSamplerDescriptorSlot};
	};

	TST_DECLARE_HANDLE(Sampler);

	struct TST_GPU_API SamplerDesc
	{
		EFilter             minFilter{EFilter::eLinear};
		EFilter             magFilter{EFilter::eLinear};
		ESamplerMipmapMode  mipmapMode{ESamplerMipmapMode::eLinear};
		ESamplerAddressMode addressModeU{ESamplerAddressMode::eRepeat};
		ESamplerAddressMode addressModeV{ESamplerAddressMode::eRepeat};
		ESamplerAddressMode addressModeW{ESamplerAddressMode::eRepeat};
	};
	#pragma endregion

	#pragma region shader

	static_assert(std::is_same_v<Flags<EShaderStageBits>::MaskType, uint8>);

	struct TST_GPU_API ShaderData
	{
		vk::ShaderEXT shader{nullptr};

		EShaderStageBits  stage{EShaderStageBits::eVertex};
		EShaderStageFlags nextStage{0u}; // The set of valid stages that could be next
	};

	TST_DECLARE_HANDLE(Shader);

	struct TST_GPU_API ShaderDesc
	{
		const void *code{nullptr};
		uint64      codeSize{0u};

		EShaderStageBits  stage{EShaderStageBits::eVertex};
		EShaderStageFlags nextStage{0u}; // The set of valid stages that could be next
	};

	#pragma endregion

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

	class CommandPool;
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

		auto acquire(BufferHandle p_handle) -> void { _acquireBuffer(p_handle); }
		auto release(BufferHandle p_handle) -> void { _releaseBuffer(p_handle); }
		auto isValid(BufferHandle p_handle) const -> bool { return _isBufferValid(p_handle); }
		auto getData(BufferHandle p_handle) const -> const BufferData * { return getBufferData(p_handle); }
		auto getData(BufferHandle p_handle) -> BufferData * { return getBufferData(p_handle); }

		auto acquire(TextureHandle p_handle) -> void { _acquireTexture(p_handle); }
		auto release(TextureHandle p_handle) -> void { _releaseTexture(p_handle); }
		auto isValid(TextureHandle p_handle) const -> bool { return _isTextureValid(p_handle); }
		auto getData(TextureHandle p_handle) const -> const TextureData * { return getTextureData(p_handle); }
		auto getData(TextureHandle p_handle) -> TextureData * { return getTextureData(p_handle); }

		auto acquire(SamplerHandle p_handle) -> void { _acquireSampler(p_handle); }
		auto release(SamplerHandle p_handle) -> void { _releaseSampler(p_handle); }
		auto isValid(SamplerHandle p_handle) const -> bool { return _isSamplerValid(p_handle); }
		auto getData(SamplerHandle p_handle) const -> const SamplerData * { return getSamplerData(p_handle); }
		auto getData(SamplerHandle p_handle) -> SamplerData * { return getSamplerData(p_handle); }

		auto acquire(ShaderHandle p_handle) -> void { _acquireShader(p_handle); }
		auto release(ShaderHandle p_handle) -> void { _releaseShader(p_handle); }
		auto isValid(ShaderHandle p_handle) const -> bool { return _isShaderValid(p_handle); }
		auto getData(ShaderHandle p_handle) const -> const ShaderData * { return getShaderData(p_handle); }
		auto getData(ShaderHandle p_handle) -> ShaderData * { return getShaderData(p_handle); }

		#pragma region buffers

		[[nodiscard]] auto createBuffer(const BufferDesc &p_desc) -> Ref<Device, BufferHandle>;

		auto getBufferData(BufferHandle p_handle) const -> const BufferData * { return m_bufferPool.getData(p_handle); }
		auto getBufferData(BufferHandle p_handle) -> BufferData * { return m_bufferPool.getData(p_handle); }
		auto uploadBufferData(BufferHandle p_handle, const void *p_data, uint64 p_size, uint64 p_offset = 0u) -> void; // Uploads data to a host-visible buffer

		#pragma endregion

		#pragma region textures

		[[nodiscard]] auto createTexture(const TextureDesc &p_desc) -> Ref<Device, TextureHandle>;

		auto getTextureData(TextureHandle p_handle) const -> const TextureData * { return m_texturePool.getData(p_handle); }
		auto getTextureData(TextureHandle p_handle) -> TextureData * { return m_texturePool.getData(p_handle); }

		// This is actually different from TextureData::shaderReadHeapID.
		// Because TextureData::shaderReadHeapID is relative to the start of the image descriptor block and not the resource heap as a whole
		auto getTextureShaderReadHeapSlot(TextureHandle p_handle) const -> uint32;
		auto getTextureStorageHeapSlot(TextureHandle p_handle) const -> uint32;

		#pragma endregion

		#pragma region samplers

		[[nodiscard]] auto createSampler(const SamplerDesc &p_desc) -> Ref<Device, SamplerHandle>;

		auto getSamplerData(SamplerHandle p_handle) const -> const SamplerData * { return m_samplerPool.getData(p_handle); }
		auto getSamplerData(SamplerHandle p_handle) -> SamplerData * { return m_samplerPool.getData(p_handle); }

		#pragma endregion

		#pragma region shaders

		[[nodiscard]] auto createShader(const ShaderDesc &p_desc) -> Ref<Device, ShaderHandle>;

		auto getShaderData(ShaderHandle p_handle) const -> const ShaderData * { return m_shaderPool.getData(p_handle); }
		auto getShaderData(ShaderHandle p_handle) -> ShaderData * { return m_shaderPool.getData(p_handle); }

		#pragma endregion

		auto createCommandPool(EQueueType p_queue_type, ECommandPoolFlags p_pool_flags) -> CommandPool;

		auto executeCommandLists(const InitialiserList<const CommandList> &p_command_lists, const InitialiserList<const vk::SemaphoreSubmitInfo> &p_wait_semaphores = {},
								 const InitialiserList<const vk::SemaphoreSubmitInfo> &p_signal_semaphores = {}, vk::Fence p_signal_fence = nullptr) const -> void;

		[[nodiscard]] auto createTimelineSemaphore(uint64 p_initial_value = 0u) const -> vk::Semaphore;
		auto waitForTimelineSemaphores(const InitialiserList<const vk::Semaphore> &p_semaphores, const InitialiserList<const uint64> &p_target_values) const -> void;

		auto waitIdle() const -> void;

	private:
		auto _destroyBuffer(BufferData *p_data) -> void;
		auto _destroyTexture(TextureData *p_data) -> void;
		auto _destroySampler(SamplerData *p_data) -> void;
		auto _destroyShader(ShaderData *p_data) -> void;

		auto _acquireBuffer(BufferHandle p_handle) -> void { m_bufferPool.incRef(p_handle); }
		auto _releaseBuffer(BufferHandle p_handle) -> void { _destroyBuffer(m_bufferPool.decRef(p_handle)); }
		auto _isBufferValid(BufferHandle p_handle) const -> bool { return m_bufferPool.isValid(p_handle); }

		auto _acquireTexture(TextureHandle p_handle) -> void { m_texturePool.incRef(p_handle); }
		auto _releaseTexture(TextureHandle p_handle) -> void { _destroyTexture(m_texturePool.decRef(p_handle)); }
		auto _isTextureValid(TextureHandle p_handle) const -> bool { return m_texturePool.isValid(p_handle); }

		auto _acquireSampler(SamplerHandle p_handle) -> void { m_samplerPool.incRef(p_handle); }
		auto _releaseSampler(SamplerHandle p_handle) -> void { _destroySampler(m_samplerPool.decRef(p_handle)); }
		auto _isSamplerValid(SamplerHandle p_handle) const -> bool { return m_samplerPool.isValid(p_handle); }

		auto _acquireShader(ShaderHandle p_handle) -> void { m_shaderPool.incRef(p_handle); }
		auto _releaseShader(ShaderHandle p_handle) -> void { _destroyShader(m_shaderPool.decRef(p_handle)); }
		auto _isShaderValid(ShaderHandle p_handle) const -> bool { return m_shaderPool.isValid(p_handle); }

		static auto getVulkanShaderStages(EShaderStageFlags p_stages) -> vk::ShaderStageFlags;

		Pool<BufferTag, BufferData>   m_bufferPool;
		Pool<TextureTag, TextureData> m_texturePool;
		Pool<SamplerTag, SamplerData> m_samplerPool;
		Pool<ShaderTag, ShaderData>   m_shaderPool;

		UniquePtr<Instance>       m_instance{nullptr};
		UniquePtr<PhysicalDevice> m_physicalDevice{nullptr};
		UniquePtr<LogicalDevice>  m_device{nullptr};

		UniquePtr<Allocator>     m_allocator{nullptr};
		UniquePtr<DeletionQueue> m_deletionQueue;

		UniquePtr<ResourceDescriptorHeap> m_resourceHeap{nullptr};
		UniquePtr<SamplerDescriptorHeap>  m_samplerHeap{nullptr};

		friend class CommandList;
	};

	using BufferRef  = Ref<Device, BufferHandle>;
	using TextureRef = Ref<Device, TextureHandle>;
	using SamplerRef = Ref<Device, SamplerHandle>;
	using ShaderRef  = Ref<Device, ShaderHandle>;
}
