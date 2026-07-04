#include "toast_gpu/vk/vk_command_buffer.hpp"

#include "toast_gpu/vk/vk_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

#include "toast_gpu/vk/vk_shader.hpp"

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

	auto VKCommandBuffer::bindIndexBuffer(const VKBuffer &p_buffer, uint64 p_offset, EIndexType p_index_type) -> void
	{
		m_commandBuffer.bindIndexBuffer(p_buffer.getBuffer(), p_offset, static_cast<vk::IndexType>(p_index_type));
	}

	auto VKCommandBuffer::drawIndexed(uint32 p_index_count, uint32 p_instance_count, uint32 p_first_index, int32 p_vertex_offset, uint32 p_first_instance) const -> void
	{
		m_commandBuffer.drawIndexed(p_index_count, p_instance_count, p_first_index, p_vertex_offset, p_first_instance);
	}

	auto VKCommandBuffer::bindShaders(const InitialiserList<const VKDynamicShader *> &p_shaders) -> void
	{
		// Apparently, if certain shader features are enabled, you have to specify all shader stages even if they are unused...
		std::unordered_map<vk::ShaderStageFlagBits, vk::ShaderEXT> shader_stages_map{
			{vk::ShaderStageFlagBits::eVertex, nullptr},
			// {vk::ShaderStageFlagBits::eTessellationControl, nullptr},
			// {vk::ShaderStageFlagBits::eTessellationEvaluation, nullptr},
			// {vk::ShaderStageFlagBits::eGeometry, nullptr},
			{vk::ShaderStageFlagBits::eTaskEXT, nullptr},
			{vk::ShaderStageFlagBits::eMeshEXT, nullptr},
			{vk::ShaderStageFlagBits::eFragment, nullptr}
		};

		for (const auto shader: p_shaders)
			shader_stages_map[shader->getStage()] = shader->getShader();

		const auto shader_stages{shader_stages_map | std::views::keys | std::ranges::to<std::vector>()};
		const auto shaders{shader_stages_map | std::views::values | std::ranges::to<std::vector>()};
		m_commandBuffer.bindShadersEXT(shader_stages, shaders);
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

	auto VKCommandBuffer::setPrimitiveTopology(EPrimitiveTopology p_primitive_topology) -> void
	{
		m_commandBuffer.setPrimitiveTopologyEXT(static_cast<vk::PrimitiveTopology>(p_primitive_topology));
	}

	auto VKCommandBuffer::setCullMode(ECullMode p_cull_mode) -> void
	{
		m_commandBuffer.setCullModeEXT(static_cast<vk::CullModeFlagBits>(p_cull_mode));
	}

	auto VKCommandBuffer::setFrontFace(EFrontFace p_front_face) -> void
	{
		m_commandBuffer.setFrontFaceEXT(static_cast<vk::FrontFace>(p_front_face));
	}

	auto VKCommandBuffer::setPolygonMode(EPolygonMode p_polygon_mode) -> void
	{
		m_commandBuffer.setPolygonModeEXT(static_cast<vk::PolygonMode>(p_polygon_mode));
	}

	auto VKCommandBuffer::setDepthTestEnable(bool32 p_enable) -> void
	{
		m_commandBuffer.setDepthTestEnableEXT(p_enable);
	}

	auto VKCommandBuffer::setDepthWriteEnable(bool32 p_enable) -> void
	{
		m_commandBuffer.setDepthWriteEnableEXT(p_enable);
	}

	auto VKCommandBuffer::setDepthCompareOp(ECompareOp p_compare_op) -> void
	{
		m_commandBuffer.setDepthCompareOpEXT(static_cast<vk::CompareOp>(p_compare_op));
	}

	auto VKCommandBuffer::setStencilTestEnable(bool32 p_enable) -> void
	{
		m_commandBuffer.setStencilTestEnableEXT(p_enable);
	}
}
