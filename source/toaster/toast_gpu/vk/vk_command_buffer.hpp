#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKCommandBuffer
	{
	public:
		VKCommandBuffer(VKGPUContext *p_ctx, vk::QueueFlagBits p_queue_type, bool p_fence_signaled = false);

		auto begin() -> void;
		auto end() -> void;
		auto submit(vk::PipelineStageFlags2                           p_wait_stage_mask   = vk::PipelineStageFlagBits2::eNone,
					const std::initializer_list<const vk::Semaphore> &p_wait_semaphores   = {},
					const std::initializer_list<const vk::Semaphore> &p_signal_semaphores = {}) -> void;

		auto getVulkanCommandBuffer() -> vk::raii::CommandBuffer &;
		auto getWaitFence() -> vk::raii::Fence &;

		auto waitForFence() -> void;
		auto resetFence() -> void;

		auto resetCommandBuffer() -> void;

	private:
		VKGPUContext *m_ctx{nullptr};

		vk::raii::CommandBuffer m_commandBuffer{nullptr};
		vk::raii::Fence         m_waitFence{nullptr};

		vk::QueueFlagBits m_queueType{vk::QueueFlagBits::eGraphics};
	};

	class VKCommandBufferPFF
	{
	public:
		VKCommandBufferPFF(VKGPUContext *p_ctx, vk::QueueFlagBits p_queue_type, uint32 p_frames_in_flight, bool p_fence_signaled = false);

		auto begin(uint32 p_frame_index) -> void;
		auto end(uint32 p_frame_index) -> void;
		auto submit(uint32                                            p_frame_index, vk::PipelineStageFlags2 p_wait_stage_mask = vk::PipelineStageFlagBits2::eNone,
					const std::initializer_list<const vk::Semaphore> &p_wait_semaphores                                        = {},
					const std::initializer_list<const vk::Semaphore> &p_signal_semaphores                                      = {}) -> void;

		auto getVulkanCommandBuffer(uint32 p_frame_index) -> vk::raii::CommandBuffer &;
		auto getWaitFence(uint32 p_frame_index) -> vk::raii::Fence &;

		auto waitForFence(uint32 p_frame_index) -> void;
		auto resetFence(uint32 p_frame_index) -> void;

		auto resetCommandBuffer(uint32 p_frame_index) -> void;

	private:
		VKGPUContext *m_ctx{nullptr};

		std::vector<vk::raii::CommandBuffer> m_commandBuffers;
		std::vector<vk::raii::Fence>         m_waitFences;

		vk::QueueFlagBits m_queueType{vk::QueueFlagBits::eGraphics};
		uint32            m_framesInFlightCount{0u};
	};
}
