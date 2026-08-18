#pragma once

#include "descriptor_heap.hpp"
#include "toast_lib/pool.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API TextureData
	{
		vk::Extent3D extent{};

		vk::Image     image{nullptr};
		VmaAllocation allocation{nullptr};
		vk::ImageView imageView{nullptr}; // Optional

		vk::ImageUsageFlags usageFlags{};
		vk::Format          format{vk::Format::eUndefined};
		vk::ImageLayout     layout{vk::ImageLayout::eUndefined};
		vk::ImageType       imageType{vk::ImageType::e2D};

		uint32 layerCount{1u};
		uint32 mipLevels{1u};

		uint32 shaderReadHeapID{invalidImageDescriptorSlot};
		uint32 storageHeapID{invalidImageDescriptorSlot};
	};

	TST_DECLARE_HANDLE(Texture);

	struct TST_GPU_API TextureDesc
	{
		vk::Extent3D        extent{};
		vk::ImageUsageFlags usageFlags{};
		vk::Format          format{vk::Format::eUndefined};
		vk::ImageType       imageType{vk::ImageType::e2D};
		uint32              layerCount{1u};
		uint32              mipLevels{1u};
	};

	class TST_GPU_API TextureManager
	{
	public:
		TextureManager(LogicalDevice &p_device, Allocator &p_allocator, ResourceDescriptorHeap &p_resource_heap);
		~TextureManager();

		[[nodiscard]] auto createTexture(const TextureDesc &p_desc) -> TextureHandle;
		auto               destroyTexture(TextureHandle p_handle) -> void; // Defers deletion of the texture and all associated heap registers

		// Only creates the heap ids. To actually set the data inside of them look at
		auto createTextureShaderReadHeapID(TextureHandle p_handle) -> void;
		auto createTextureStorageHeapID(TextureHandle p_handle) -> void;

		// The target layout is the layout the image will have to be in when accessed as a descriptor
		auto setTextureShaderReadHeapInfo(TextureHandle p_handle, const vk::ImageViewCreateInfo &p_image_view_create_info, vk::ImageLayout p_target_layout) -> void;
		auto setTextureStorageHeapInfo(TextureHandle p_handle, const vk::ImageViewCreateInfo &p_image_view_create_info, vk::ImageLayout p_target_layout) -> void;

		auto isValid(TextureHandle p_handle) const -> bool;
		auto getData(TextureHandle p_handle) -> TextureData *;

	private:
		NonOwningPtr<LogicalDevice>          m_device{nullptr};
		NonOwningPtr<Allocator>              m_allocator{nullptr};
		NonOwningPtr<ResourceDescriptorHeap> m_resourceHeap{nullptr};

		Pool<TextureTag, TextureData> m_pool;
	};
}
