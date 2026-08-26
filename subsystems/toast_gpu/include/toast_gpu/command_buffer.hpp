#pragma once

#include "gpu_context.hpp"

namespace toaster::gpu
{
	// enum class EQueueType
	// {
	// 	eGraphics, eCompute, eTransfer
	// };
	//
	// struct TST_GPU_API CommandListDesc
	// {
	// 	EQueueType queueType{EQueueType::eGraphics};
	// };
	//
	// class TST_GPU_API CommandList
	// {
	// public:
	// 	CommandList(GPUContext &p_gpu_ctx, const CommandListDesc &p_desc);
	// 	~CommandList();
	//
	// 	auto begin() -> void;
	// 	auto end() -> void;
	//
	// 	auto copyBuffer(BufferHandle p_src, BufferHandle p_dst, uint64 p_size, uint64 p_src_offset = 0u, uint64 p_dst_offset = 0u) -> void;
	//
	// private:
	// 	NonOwningPtr<GPUContext> m_gpuCtx{nullptr};
	// 	vk::CommandBuffer        m_cmd{nullptr};
	// };
	//
	// class TST_GPU_API CommandQueue
	// {
	// public:
	// 	// auto submit(const std::initializer_list<const CommandList& >& p_command_lists, const std::initializer_list<>) -> void;
	// private:
	// 	vk::Queue m_queue{nullptr};
	// };
}
