#pragma once

#include "device.hpp"

namespace toaster::gpu
{
	// To create this, go to Device::createCommandList
	class TST_GPU_API CommandList
	{
	public:
		// Easier than managing it in the class itself
		CommandList(Device &p_device, vk::CommandBuffer p_cmd);
		~CommandList();

		CommandList(const CommandList &p_other);
		CommandList(CommandList &&p_other) noexcept;
		CommandList &operator=(const CommandList &p_other);
		CommandList &operator=(CommandList &&p_other) noexcept;

		auto getCommandBuffer() const -> vk::CommandBuffer { return m_cmd; }

		// Only works if the pool was created with the reset bit enabled
		auto reset() -> void;

		auto begin() -> void;
		auto end() -> void;

		auto bindResourceHeap(const ResourceDescriptorHeap &p_resource_heap) -> void;
		auto bindSamplerHeap(const SamplerDescriptorHeap &p_sampler_heap) -> void;

		auto copyBuffer(BufferHandle p_src_buffer, BufferHandle p_dst_buffer, uint64 p_size, uint64 p_src_offset = 0u, uint64 p_dst_offset = 0u) -> void;
		auto copyBufferToTexture(BufferHandle p_src_buffer, TextureHandle p_dst_texture) -> void;

		auto transitionTextureLayout(TextureHandle p_texture, vk::ImageLayout p_dst_layout) -> void;

	private:
		NonOwningPtr<Device> m_device{nullptr};
		vk::CommandBuffer    m_cmd{nullptr};
	};
}
