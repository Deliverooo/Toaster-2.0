#pragma once

#include "toast_render.hpp"
#include "toast_gpu/vk/vk_descriptor_heap.hpp"
#include "toast_gpu/vk/vk_raw_image.hpp"

namespace toaster::render
{
	class RenderContext;

	using ImageSize = tsm::uint2;

	struct TST_RENDER_API ImageSpecInfo
	{
		ImageSize size{0u};

		vk::Format format{vk::Format::eR8G8B8A8Srgb};

		// True if the image should be able to be used as a storage image inside of shaders. Also dictates whether the storage heap id is created in the constructor
		bool32 storage{false};

		uint32 layerCount{1u};
	};

	// The ultimate image class. Ts can be used for basically anything inside the engine.
	// You can use this as a storage image, sampled image, colour attachment or a 3d cube map image
	// For functions to create images for these specialised functionalities, check the render context class
	class TST_RENDER_API Image
	{
		TST_RENDER_OBJECT
	public:
		Image(RenderContext &p_render_ctx, const ImageSpecInfo &p_spec_info);
		Image(RenderContext &p_render_ctx, const ImageSpecInfo &p_spec_info, const Buffer &p_data);
		Image(RenderContext &p_render_ctx, const gpu::RawImageHandle &p_raw_image);

		~Image();

		auto setData(const Buffer &p_data) -> void;

		auto getSpecInfo() const -> const ImageSpecInfo &;
		auto getImage() const -> const gpu::RawImageHandle &;
		auto getImage() -> gpu::RawImageHandle &;

		auto resize(ImageSize p_size) -> void;

		// Use when switching the image's usage to storage or shader read
		auto toStorageOptimal() -> void;
		auto toShaderReadOptimal() -> void;

		auto getStorageHeapID() const -> gpu::DescriptorSlot;    // Use for storage images
		auto getShaderReadHeapID() const -> gpu::DescriptorSlot; // Use for sampled images

		auto getAlignedStorageHeapID() const -> gpu::DescriptorSlot;    // Pass ts to shaders!!!!
		auto getAlignedShaderReadHeapID() const -> gpu::DescriptorSlot; // Pass ts to shaders!!!!

	private:
		ImageSpecInfo m_specInfo{};

		gpu::RawImageHandle m_image{nullptr};

		gpu::DescriptorSlot m_storageHeapID{UINT32_MAX};
		gpu::DescriptorSlot m_shaderReadHeapID{UINT32_MAX};
	};

	TST_RENDER_DEFINE_HANDLE(Image, Image)
}
