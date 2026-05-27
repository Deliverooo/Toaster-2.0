#pragma once

#include "vk_raw_image.hpp"
#include "toast_gpu/resource.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;

	class TST_GPU_API VKStorageImage : public IGPUResource
	{
		TST_GPU_OBJECT
		TST_GPU_RESOURCE(StorageImage)
	public:
		VKStorageImage(VKLogicalDevice *p_device, const ImageSpecInfo &p_spec_info);
		~VKStorageImage();

		virtual auto populateWriteDescriptor(vk::WriteDescriptorSet &p_write_descriptor, uint32 p_frame_index) -> void override;
		virtual auto getDescriptorResourceHandle(uint32 p_frame_index) -> void * override;

		auto resize(uint32 p_width, uint32 p_height) -> void;
		auto setData(void *p_data, uint64 p_size) -> void;
		auto setData(const Buffer &p_buffer) -> void;

		// If you want to work with textures and don't want to immediately set the data you might want to deffer the sampler creation
		auto createSampler(vk::ImageLayout p_override_layout = vk::ImageLayout::eUndefined) -> void; // Also creates the descriptor info

		auto getImage() const -> const RawImageHandle &;
		auto getDescriptorInfo() const -> const vk::DescriptorImageInfo &;

	private:
		RawImageHandle m_image{nullptr};

		vk::Sampler m_sampler{nullptr};
		Buffer      m_imageData;

		vk::DescriptorImageInfo m_descriptorImageInfo{};
	};

	TST_GPU_DEFINE_HANDLE(VKStorageImage, StorageImage)
}
