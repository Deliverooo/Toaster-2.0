#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::gpu
{
	VKCommandBuffer::VKCommandBuffer(VKLogicalDevice *p_device, vk::QueueFlagBits p_queue_type, bool p_fence_signaled) : m_device(p_device), m_queueType(p_queue_type)
	{
		vk::CommandBufferAllocateInfo alloc_info{};
		alloc_info.commandBufferCount = 1;
		alloc_info.commandPool        = m_device->getCommandPool(m_queueType);
		alloc_info.level              = vk::CommandBufferLevel::ePrimary;

		m_commandBuffer = std::move(m_device->getVulkanLogicalDevice().allocateCommandBuffers(alloc_info).front());

		vk::FenceCreateInfo fence_create_info{};
		fence_create_info.flags = p_fence_signaled ? vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlagBits{};
		m_waitFence             = {*m_device, fence_create_info};
	}

	auto VKCommandBuffer::begin() -> void
	{
		constexpr vk::CommandBufferBeginInfo begin_info{};
		m_commandBuffer.begin(begin_info);
	}

	auto VKCommandBuffer::end() -> void
	{
		m_commandBuffer.end();
	}

	auto VKCommandBuffer::endAndSubmit() -> void
	{
		m_commandBuffer.end();
		submit();
		waitForFence();
	}

	auto VKCommandBuffer::submit(vk::PipelineStageFlags2                           p_wait_stage_mask, const std::initializer_list<const vk::Semaphore> &p_wait_semaphores,
								 const std::initializer_list<const vk::Semaphore> &p_signal_semaphores) -> void
	{
		vk::CommandBufferSubmitInfo command_buffer_info{};
		command_buffer_info.commandBuffer = m_commandBuffer;

		std::vector<vk::SemaphoreSubmitInfo> wait_semaphore_infos;
		for (const auto &wait_semaphore: p_wait_semaphores)
		{
			vk::SemaphoreSubmitInfo &info{wait_semaphore_infos.emplace_back()};
			info.semaphore = wait_semaphore;
			info.stageMask = p_wait_stage_mask;
		}

		std::vector<vk::SemaphoreSubmitInfo> signal_semaphore_infos;
		for (const auto &signal_semaphore: p_signal_semaphores)
		{
			vk::SemaphoreSubmitInfo &info{signal_semaphore_infos.emplace_back()};
			info.semaphore = signal_semaphore;
		}

		vk::SubmitInfo2 submit_info{};
		submit_info.commandBufferInfoCount   = 1;
		submit_info.pCommandBufferInfos      = &command_buffer_info;
		submit_info.waitSemaphoreInfoCount   = wait_semaphore_infos.size();
		submit_info.pWaitSemaphoreInfos      = wait_semaphore_infos.data();
		submit_info.signalSemaphoreInfoCount = signal_semaphore_infos.size();
		submit_info.pSignalSemaphoreInfos    = signal_semaphore_infos.data();
		m_device->getQueue(m_queueType).submit2(submit_info, m_waitFence);
	}

	auto VKCommandBuffer::getVulkanCommandBuffer() -> vk::raii::CommandBuffer &
	{
		return m_commandBuffer;
	}

	auto VKCommandBuffer::getWaitFence() -> vk::raii::Fence &
	{
		return m_waitFence;
	}

	auto VKCommandBuffer::waitForFence() -> void
	{
		m_device->waitForFences({*m_waitFence});
	}

	auto VKCommandBuffer::resetFence() -> void
	{
		m_device->getVulkanLogicalDevice().resetFences(*m_waitFence);
	}

	auto VKCommandBuffer::resetCommandBuffer() -> void
	{
		m_commandBuffer.reset();
	}

	auto VKCommandBuffer::setRenderArea(const vk::Rect2D &p_area) const -> void
	{
		const vk::Extent2D rendering_extent{p_area.extent};
		const vk::Offset2D rendering_offset{p_area.offset};

		const vk::Viewport viewport{
			static_cast<float32>(rendering_offset.x),
			static_cast<float32>(rendering_offset.y),
			static_cast<float32>(rendering_extent.width),
			static_cast<float32>(rendering_extent.height),
			0.0f,
			1.0f
		};
		const vk::Rect2D scissor{rendering_offset, rendering_extent};

		m_commandBuffer.setViewportWithCountEXT(viewport);
		m_commandBuffer.setScissorWithCountEXT(scissor);
	}

	auto VKCommandBuffer::drawIndexed(uint32 p_index_count, uint32 p_instance_count, uint32 p_first_index, int32 p_vertex_offset, uint32 p_first_instance) const -> void
	{
		m_commandBuffer.drawIndexed(p_index_count, p_instance_count, p_first_index, p_vertex_offset, p_first_instance);
	}

	VKCommandBufferPFFPacked::VKCommandBufferPFFPacked(VKLogicalDevice *p_device, vk::QueueFlagBits p_queue_type, uint32 p_frames_in_flight,
													   bool p_fence_signaled) : m_device(p_device), m_queueType(p_queue_type), m_framesInFlightCount(p_frames_in_flight)
	{
		TST_ASSERT_MSG(m_framesInFlightCount > 0, "Bradar what is dis?!");

		vk::CommandBufferAllocateInfo alloc_info{};
		alloc_info.commandBufferCount = m_framesInFlightCount;
		alloc_info.commandPool        = m_device->getCommandPool(m_queueType);
		alloc_info.level              = vk::CommandBufferLevel::ePrimary;

		m_commandBuffers = m_device->getVulkanLogicalDevice().allocateCommandBuffers(alloc_info);

		for (uint32 i{0u}; i < m_framesInFlightCount; ++i)
		{
			vk::FenceCreateInfo fence_create_info{};
			fence_create_info.flags = p_fence_signaled ? vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlagBits{};
			m_waitFences.emplace_back(std::move<vk::raii::Fence>({*m_device, fence_create_info}));
		}
	}

	auto VKCommandBufferPFFPacked::begin(uint32 p_frame_index) -> void
	{
		constexpr vk::CommandBufferBeginInfo begin_info{};
		m_commandBuffers.at(p_frame_index).begin(begin_info);
	}

	auto VKCommandBufferPFFPacked::end(uint32 p_frame_index) -> void
	{
		m_commandBuffers.at(p_frame_index).end();
	}

	auto VKCommandBufferPFFPacked::submit(uint32                                            p_frame_index, vk::PipelineStageFlags2 p_wait_stage_mask,
										  const std::initializer_list<const vk::Semaphore> &p_wait_semaphores,
										  const std::initializer_list<const vk::Semaphore> &p_signal_semaphores) -> void
	{
		vk::CommandBufferSubmitInfo command_buffer_info{};
		command_buffer_info.commandBuffer = m_commandBuffers.at(p_frame_index);

		std::vector<vk::SemaphoreSubmitInfo> wait_semaphore_infos;
		for (const auto &wait_semaphore: p_wait_semaphores)
		{
			vk::SemaphoreSubmitInfo &info{wait_semaphore_infos.emplace_back()};
			info.semaphore = wait_semaphore;
			info.stageMask = p_wait_stage_mask;
		}

		std::vector<vk::SemaphoreSubmitInfo> signal_semaphore_infos;
		for (const auto &signal_semaphore: p_signal_semaphores)
		{
			vk::SemaphoreSubmitInfo &info{signal_semaphore_infos.emplace_back()};
			info.semaphore = signal_semaphore;
		}

		vk::SubmitInfo2 submit_info{};
		submit_info.commandBufferInfoCount   = 1;
		submit_info.pCommandBufferInfos      = &command_buffer_info;
		submit_info.waitSemaphoreInfoCount   = wait_semaphore_infos.size();
		submit_info.pWaitSemaphoreInfos      = wait_semaphore_infos.data();
		submit_info.signalSemaphoreInfoCount = signal_semaphore_infos.size();
		submit_info.pSignalSemaphoreInfos    = signal_semaphore_infos.data();
		m_device->getQueue(m_queueType).submit2(submit_info, m_waitFences[p_frame_index]);
	}

	auto VKCommandBufferPFFPacked::getVulkanCommandBuffer(uint32 p_frame_index) -> vk::raii::CommandBuffer &
	{
		return m_commandBuffers.at(p_frame_index);
	}

	auto VKCommandBufferPFFPacked::getWaitFence(uint32 p_frame_index) -> vk::raii::Fence &
	{
		return m_waitFences.at(p_frame_index);
	}

	auto VKCommandBufferPFFPacked::waitForFence(uint32 p_frame_index) -> void
	{
		m_device->waitForFence(*m_waitFences.at(p_frame_index));
	}

	auto VKCommandBufferPFFPacked::resetFence(uint32 p_frame_index) -> void
	{
		m_device->getVulkanLogicalDevice().resetFences(*m_waitFences.at(p_frame_index));
	}

	auto VKCommandBufferPFFPacked::resetCommandBuffer(uint32 p_frame_index) -> void
	{
		m_commandBuffers.at(p_frame_index).reset();
	}
}
