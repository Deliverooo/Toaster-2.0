#pragma once

#include "buffer_manager.hpp"
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

		void *            mapped{nullptr}; // I guess you can have mapped image memory...
		EMemoryProperties memoryProperties{EMemoryProperties::eDeviceLocal};

		DescriptorSlot shaderReadHeapID{invalidImageDescriptorSlot};
		DescriptorSlot storageHeapID{invalidImageDescriptorSlot};
	};

	TST_DECLARE_HANDLE(Texture);

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

	class TST_GPU_API TextureManager
	{
		TST_REGISTER_DEPENDENCY(Device, Device, gpuCtx)
		TST_REGISTER_DEPENDENCY(ResourceDescriptorHeap, ResourceHeap, resourceHeap)
		TST_REGISTER_DEPENDENCY(BufferManager, BufferManager, bufferManager)
	public:
		using PoolType = Pool<TextureTag, TextureData>;

		TextureManager(Device &p_gpu_ctx, ResourceDescriptorHeap &p_resource_heap, BufferManager &p_buffer_manager);
		~TextureManager();

		TextureManager(const TextureManager &)            = delete;
		TextureManager(TextureManager &&)                 = delete;
		TextureManager &operator=(const TextureManager &) = delete;
		TextureManager &operator=(TextureManager &&)      = delete;

		[[nodiscard]] auto createTexture(const TextureDesc &p_desc) -> TextureHandle;
		auto               createSharedTexture(const TextureDesc &p_desc) -> SharedHandle<TextureTag, TextureData>;
		auto               destroyTexture(TextureHandle p_handle) -> void { m_pool.destroy(p_handle); } // Secretly defers it...

		// This is actually different from TextureData::shaderReadHeapID.
		// Because TextureData::shaderReadHeapID is relative to the start of the image descriptor block and not the resource heap as a whole
		auto getTextureShaderReadHeapSlot(TextureHandle p_handle) const -> DescriptorSlot;
		auto getTextureStorageHeapSlot(TextureHandle p_handle) const -> DescriptorSlot;

		// src_layout is from TextureData::layout. If you are using the texture as a render target, the command buffer should be per frame in flight (e.g. from the swapchain)
		auto transitionTextureLayout(TextureHandle p_handle, vk::CommandBuffer p_cmd, vk::ImageLayout p_dst_layout) -> void;
		auto setTextureData(TextureHandle p_handle, vk::CommandBuffer p_cmd, const void *p_data, uint64 p_size, uint64 p_offset = 0u) -> void;

		auto isValid(TextureHandle p_handle) const -> bool;
		auto getData(TextureHandle p_handle) const -> const TextureData *;
		auto getData(TextureHandle p_handle) -> TextureData *;

		auto getPoolData(uint32 p_id) -> TextureData * { return &m_pool._data[p_id]; }
		auto getPoolData(uint32 p_id) const -> const TextureData * { return &m_pool._data[p_id]; }

	private:
		auto _transitionTextureLayout(TextureData *p_data, vk::CommandBuffer p_cmd, vk::ImageLayout p_dst_layout) -> void;

		// All the required dependencies to destroy a texture. May be called after this class is destroyed due to the deletion queue
		static auto _destroyTextureData(const TextureData &p_texture_data, const Device &p_gpu_ctx, ResourceDescriptorHeap &p_resource_heap) -> void;

		PoolType m_pool;
	};

	using SharedTexture = SharedHandle<TextureTag, TextureData>;
}
