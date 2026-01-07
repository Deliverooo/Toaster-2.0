#pragma once

#include <vulkan/vulkan.hpp>

#include "system_types.h"

namespace toaster::gpu
{
	class GPUContext;

	class Texture
	{
	public:
		Texture(GPUContext *p_ctx, const std::string &p_path);
		~Texture();

		void updateDescriptor();
		const vk::DescriptorImageInfo *getDescriptorImageInfo() const;

	private:
		vk::DescriptorImageInfo m_descriptor;

		GPUContext *m_gpuContext;

		vk::Image        m_image;
		vk::ImageLayout  m_imageLayout;
		vk::DeviceMemory m_deviceMemory;
		vk::ImageView    m_imageView;
		vk::Sampler      m_sampler;
		int32            m_width;
		int32            m_height;
	};
}
