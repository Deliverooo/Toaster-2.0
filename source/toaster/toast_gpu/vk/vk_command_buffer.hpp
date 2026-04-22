#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	class VKGPUContext;

	class VKCommandBuffer
	{
	public:
		VKCommandBuffer(VKGPUContext *p_ctx, vk::QueueFlagBits p_queue_type);

		auto begin() -> void;
		auto end() -> void;
		auto submit() -> void;

		auto getVulkanCommandBuffer() -> vk::raii::CommandBuffer &;
		auto getWaitFence() -> vk::raii::Fence &;

	private:
		auto _getCommandPool(vk::QueueFlagBits p_queue_type) const -> vk::raii::CommandPool &;
		auto _getQueue(vk::QueueFlagBits p_queue_type) const -> vk::raii::Queue &;

		VKGPUContext *m_ctx{nullptr};

		vk::raii::CommandBuffer m_commandBuffer{nullptr};
		vk::raii::Fence         m_waitFence{nullptr};

		vk::QueueFlagBits m_queueType{vk::QueueFlagBits::eGraphics};
	};
}
