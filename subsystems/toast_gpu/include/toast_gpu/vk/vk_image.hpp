#pragma once

#include "vk_descriptor_heap.hpp"
#include "vk_raw_image.hpp"

namespace toaster::gpu
{
	using ImageSize = tsm::uint2;

	struct TST_GPU_API ImageSpecInfo
	{
		enum EConfigFlags : uint32
		{
			eComputeMipLevels = UINT32_MAX, eCubeMap = 6u
		};

		ImageSize size{0u};

		vk::Format format{vk::Format::eR8G8B8A8Srgb};

		// True if the image should be able to be used as a storage image inside of shaders. Also dictates whether the storage heap id is created in the constructor
		bool32 storage{false};
		bool32 hostAccess{false};

		uint32 mipLevels{0u};
		uint32 layerCount{1u};
	};

	// The ultimate image class. Ts can be used for basically anything inside the engine.
	// You can use this as a storage image, sampled image, colour attachment or a 3d cube map image
	// For functions to create images for these specialised functionalities, check the render context class
	class TST_GPU_API VKImage
	{
		TST_GPU_OBJECT
	public:
		VKImage(VKGPUContext &p_gpu_ctx, const ImageSpecInfo &p_spec_info);
		VKImage(VKGPUContext &p_gpu_ctx, const ImageSpecInfo &p_spec_info, const toaster::Buffer &p_data);
		VKImage(VKGPUContext &p_gpu_ctx, const RawImageHandle &p_raw_image);
		~VKImage();

		auto setData(const toaster::Buffer &p_data) -> void;

		auto generateMipmaps() -> void;

		auto getSpecInfo() const -> const ImageSpecInfo &;
		auto getImage() const -> const RawImageHandle &;
		auto getImage() -> RawImageHandle &;

		auto resize(ImageSize p_size) -> void;

		// Use when switching the image's usage to storage or shader read
		auto toStorageOptimal() -> void;
		auto toShaderReadOptimal() -> void;

		auto getStorageDescriptorSlot() const -> DescriptorSlot;    // Use for storage images
		auto getShaderReadDescriptorSlot() const -> DescriptorSlot; // Use for sampled images

		auto getStorageHeapID() const -> DescriptorSlot;    // Pass ts to shaders!!!!
		auto getShaderReadHeapID() const -> DescriptorSlot; // Pass ts to shaders!!!!

		auto createMipDescriptorSlot(uint32 p_level) -> void;

		auto getMipStorageDescriptorSlot(uint32 p_level) const -> DescriptorSlot;
		auto getMipShaderReadDescriptorSlot(uint32 p_level) const -> DescriptorSlot;

		auto getMipStorageHeapID(uint32 p_level) const -> DescriptorSlot;
		auto getMipShaderReadHeapID(uint32 p_level) const -> DescriptorSlot;

	private:
		ImageSpecInfo m_specInfo{};

		RawImageHandle m_image{nullptr};

		DescriptorSlot m_storageDescriptorSlot{UINT32_MAX};
		DescriptorSlot m_shaderDescriptorSlot{UINT32_MAX};

		std::unordered_map<uint32, DescriptorSlot> m_perMipStorageDescriptorSlots;
		std::unordered_map<uint32, DescriptorSlot> m_perMipShaderReadDescriptorSlots;
	};

	TST_GPU_DEFINE_HANDLE(VKImage, Image)
}
