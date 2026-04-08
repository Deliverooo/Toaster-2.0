#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"
#include "../resource.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKUniformBuffer final : public IGPUResource
	{
	public:
		VKUniformBuffer(VKGPUContext *p_ctx, uint64 p_size);

		vk::raii::Buffer &      getBuffer();
		vk::raii::DeviceMemory &getBufferMemory();

		const vk::DescriptorBufferInfo &getDescriptorInfo() const;

		void setData(void *p_data, uint64 p_size, uint64 p_offset);

		void *mapMemory(uint64 p_size, uint64 p_offset);
		void  unmapMemory();

		EGPUResourceType getResourceType() const override;

	private:
		VKGPUContext *m_ctx{nullptr};

		vk::raii::Buffer         m_buffer{nullptr};
		vk::raii::DeviceMemory   m_bufferMemory{nullptr};
		vk::DescriptorBufferInfo m_descriptorInfo{};
	};

	// Uniform buffer, but it's per frame in flight
	// Ts is easier to use than allocating them manually, so it just makes things more manageable
	// Typically you wouldn't really use a single uniform buffer anyway, so use this instead.
	class VKUniformBufferPFF final : public IGPUResource
	{
	public:
		VKUniformBufferPFF(VKGPUContext *p_ctx, uint64 p_size, uint32 p_frames_in_flight);

		RefPtr<VKUniformBuffer> getUBO(uint32 p_frame_index);
		void                    setUBO(uint32 p_frame_index, const RefPtr<VKUniformBuffer> &p_uniform_buffer);

		std::vector<RefPtr<VKUniformBuffer> >::iterator begin();
		std::vector<RefPtr<VKUniformBuffer> >::iterator end();

		EGPUResourceType getResourceType() const override;

		std::vector<void *> mapMemory(uint64 p_size, uint64 p_offset);
		void                unmapMemory();

	private:
		std::vector<RefPtr<VKUniformBuffer> > m_uniformBuffers;
		uint32                                m_framesInFlightCount{0u};
	};
}
