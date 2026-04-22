#include "vk_command_buffer.hpp"

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKCommandBuffer::VKCommandBuffer(VKGPUContext *p_ctx, vk::QueueFlagBits p_queue_type) : m_ctx(p_ctx), m_queueType(p_queue_type)
	{
		vk::CommandBufferAllocateInfo alloc_info{};
		alloc_info.commandBufferCount = 1;
		alloc_info.commandPool        = _getCommandPool(m_queueType);
		alloc_info.level              = vk::CommandBufferLevel::ePrimary;

		m_commandBuffer = std::move(m_ctx->getLogicalDevice()->getVulkanLogicalDevice().allocateCommandBuffers(alloc_info).front());

		m_waitFence = {*m_ctx->getLogicalDevice(), vk::FenceCreateInfo{}};
	}

	auto VKCommandBuffer::begin() -> void
	{
		const vk::CommandBufferBeginInfo begin_info{};
		m_commandBuffer.begin(begin_info);
	}

	auto VKCommandBuffer::end() -> void
	{
		m_commandBuffer.end();
	}

	auto VKCommandBuffer::submit() -> void
	{
		vk::CommandBufferSubmitInfo command_buffer_info{};
		command_buffer_info.commandBuffer = m_commandBuffer;

		vk::SubmitInfo2 submit_info{};
		submit_info.commandBufferInfoCount = 1;
		submit_info.pCommandBufferInfos    = &command_buffer_info;
		_getQueue(m_queueType).submit2(submit_info, m_waitFence);

		m_ctx->getLogicalDevice()->waitForFence(*m_waitFence);
	}

	auto VKCommandBuffer::getVulkanCommandBuffer() -> vk::raii::CommandBuffer &
	{
		return m_commandBuffer;
	}

	auto VKCommandBuffer::getWaitFence() -> vk::raii::Fence &
	{
		return m_waitFence;
	}

	auto VKCommandBuffer::_getCommandPool(vk::QueueFlagBits p_queue_type) const -> vk::raii::CommandPool &
	{
		switch (p_queue_type)
		{
			case vk::QueueFlagBits::eGraphics: return m_ctx->getLogicalDevice()->getGraphicsCommandPool();
				break;
			case vk::QueueFlagBits::eCompute: return m_ctx->getLogicalDevice()->getComputeCommandPool();
				break;
			case vk::QueueFlagBits::eTransfer: return m_ctx->getLogicalDevice()->getTransferCommandPool();
				break;
			// case vk::QueueFlagBits::eSparseBinding:
			// case vk::QueueFlagBits::eProtected:
			// case vk::QueueFlagBits::eVideoDecodeKHR:
			// case vk::QueueFlagBits::eVideoEncodeKHR:
			// case vk::QueueFlagBits::eOpticalFlowNV:
			// case vk::QueueFlagBits::eDataGraphARM:
			default: break;
		}
		TST_ASSERT_MSG(false, "Unsupported queue");
		return m_ctx->getLogicalDevice()->getGraphicsCommandPool();
	}

	auto VKCommandBuffer::_getQueue(vk::QueueFlagBits p_queue_type) const -> vk::raii::Queue &
	{
		switch (p_queue_type)
		{
			case vk::QueueFlagBits::eGraphics: return m_ctx->getLogicalDevice()->getGraphicsQueue();
				break;
			case vk::QueueFlagBits::eCompute: return m_ctx->getLogicalDevice()->getComputeQueue();
				break;
			case vk::QueueFlagBits::eTransfer: return m_ctx->getLogicalDevice()->getTransferQueue();
				break;
			// case vk::QueueFlagBits::eSparseBinding:
			// case vk::QueueFlagBits::eProtected:
			// case vk::QueueFlagBits::eVideoDecodeKHR:
			// case vk::QueueFlagBits::eVideoEncodeKHR:
			// case vk::QueueFlagBits::eOpticalFlowNV:
			// case vk::QueueFlagBits::eDataGraphARM:

			default: break;
		}

		TST_ASSERT_MSG(false, "Unsupported queue");
		return m_ctx->getLogicalDevice()->getGraphicsQueue();
	}
}
