#pragma once

#include "allocator.hpp"
#include "deletion_queue.hpp"

#include "gpu_enums.hpp"

#include "buffer.hpp"
#include "texture.hpp"
#include "sampler.hpp"
#include "shader.hpp"

namespace toaster::gpu
{
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

		#define TST_REGISTER_RESOURCE_GETTERS(__tag, __tag2) auto get##__tag##Data(__tag##Handle p_handle) -> __tag##Data* { return m_##__tag2##Manager.getData(p_handle); }\
		auto get##__tag##Data(__tag##Handle p_handle) const -> const __tag##Data* { return m_##__tag2##Manager.getData(p_handle); }

		[[nodiscard]] auto createBuffer(const BufferDesc &p_desc) -> BufferRef;
		TST_REGISTER_RESOURCE_GETTERS(Buffer, buffer)

		auto uploadBufferData(BufferHandle p_handle, const void *p_data, uint64 p_size, uint64 p_offset = 0u) -> void; // Uploads data to a host-visible buffer

		[[nodiscard]] auto createTexture(const TextureDesc &p_desc) -> TextureRef;
		TST_REGISTER_RESOURCE_GETTERS(Texture, texture)

		[[nodiscard]] auto createSampler(const SamplerDesc &p_desc) -> SamplerRef;
		TST_REGISTER_RESOURCE_GETTERS(Sampler, sampler)

		[[nodiscard]] auto createShader(const ShaderDesc &p_desc) -> ShaderRef;
		TST_REGISTER_RESOURCE_GETTERS(Shader, shader)

		#undef TST_REGISTER_RESOURCE_GETTERS

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

		static auto getVulkanShaderStages(EShaderStageFlags p_stages) -> vk::ShaderStageFlags;

		ResourceManager<BufferTag, BufferData> m_bufferManager;

		ResourceManager<TextureTag, TextureData> m_textureManager;
		ResourceManager<SamplerTag, SamplerData> m_samplerManager;
		ResourceManager<ShaderTag, ShaderData>   m_shaderManager;

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
