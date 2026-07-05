#pragma once

#include "../toast_gpu.hpp"
#include "../gpu_enums.hpp"

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	class VKLogicalDevice;

	class VKDynamicShader;

	class VKBuffer;

	class TST_GPU_API VKCommandBuffer
	{
		TST_GPU_OBJECT
	public:
		VKCommandBuffer(VKLogicalDevice *p_device, vk::QueueFlagBits p_queue_type, bool p_fence_signaled = false);

		auto begin() -> void;
		auto end() -> void;
		auto endAndSubmit() -> void; // Used for one-time commands or basic things
		auto submit(vk::PipelineStageFlags2                           p_wait_stage_mask   = vk::PipelineStageFlagBits2::eNone,
					const std::initializer_list<const vk::Semaphore> &p_wait_semaphores   = {},
					const std::initializer_list<const vk::Semaphore> &p_signal_semaphores = {}) -> void;

		auto getVulkanCommandBuffer() -> vk::raii::CommandBuffer &;
		auto getWaitFence() -> vk::raii::Fence &;

		auto waitForFence() -> void;
		auto resetFence() -> void;

		auto resetCommandBuffer() -> void;

		#pragma region vulkan wrappers

		template<typename TConstants>
		auto pushData(const TConstants &p_data, uint64 p_offset = 0u) const -> void
		{
			vk::PushDataInfoEXT push_data_info{};
			push_data_info.offset       = p_offset;
			push_data_info.data.address = &p_data;
			push_data_info.data.size    = sizeof(TConstants);

			m_commandBuffer.pushDataEXT(push_data_info);
		}

		auto bindIndexBuffer(const VKBuffer &p_buffer, uint64 p_offset = 0u, EIndexType p_index_type = EIndexType::eUint32) -> void;

		auto drawIndexed(uint32 p_index_count, uint32 p_instance_count = 1u, uint32 p_first_index = 0u, int32 p_vertex_offset = 0u,
						 uint32 p_first_instance                       = 0u) const -> void;
		#pragma endregion

		#pragma region render logic

		auto bindShaders(const InitialiserList<const VKDynamicShader *> &p_shaders) -> void;

		auto setRenderArea(const vk::Rect2D &p_area) const -> void;

		auto setPrimitiveTopology(EPrimitiveTopology p_primitive_topology) -> void;

		auto setCullMode(ECullMode p_cull_mode) -> void;
		auto setFrontFace(EFrontFace p_front_face) -> void;

		auto setPolygonMode(EPolygonMode p_polygon_mode) -> void;

		auto setDepthTestEnable(bool32 p_enable) -> void;
		auto setDepthWriteEnable(bool32 p_enable) -> void;
		auto setDepthCompareOp(ECompareOp p_compare_op) -> void;

		auto setStencilTestEnable(bool32 p_enable) -> void;

		auto setColourBlendEnable(const InitialiserList<const bool32> &p_enables) -> void;
		auto setColourWriteMask(const InitialiserList<const vk::ColorComponentFlags> &p_write_masks) -> void;

		auto setRasterizationSamples(ESampleCount p_sample_count) -> void;

		auto setPrimitiveRestartEnable(bool32 p_enable) -> void;
		auto setDepthClampEnable(bool32 p_enable) -> void;
		auto setDepthBiasEnable(bool32 p_enable) -> void;
		auto setRasterizerDiscardEnable(bool32 p_enable) -> void;

		auto setAlphaToCoverageEnable(bool32 p_enable) -> void;

		auto setLineWidth(float32 p_width) -> void;

		#pragma endregion

		operator vk::CommandBuffer() const { return *m_commandBuffer; }

	private:
		vk::raii::CommandBuffer m_commandBuffer{nullptr};
		vk::raii::Fence         m_waitFence{nullptr};

		vk::QueueFlagBits m_queueType{vk::QueueFlagBits::eGraphics};
	};

	TST_GPU_DEFINE_HANDLE(VKCommandBuffer, CommandBuffer);
}
