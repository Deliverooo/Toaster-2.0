#pragma once

#include "api.hpp"

namespace toaster::gpu::frame
{
	struct TST_GPU_API FrameContextDesc
	{
		uint32 maxFramesInFlight{3u}; // Same as GPUContextDesc::maxConcurrentSwapchainWorkloads...
	};

	// Must be called after initGPUContext(...)
	auto TST_GPU_API initFrameContext(const FrameContextDesc &p_desc) -> void;
	// Must be called before shutdownGPUContext()
	auto TST_GPU_API shutdownFrameContext() -> void;

	auto TST_GPU_API getMaxFramesInFlight() -> uint32; // You should be tracking this yourself, but here.

	// Waits on timeline semaphores and resets previous command lists
	auto TST_GPU_API beginFrame(uint32 p_frame_index) -> void;

	// Returns true if successful, false if the present operation was unsuccessful. Recreate if false
	// Prefer over the standard on from <api.hpp> because this has automatic timeline semaphore tracking
	auto TST_GPU_API submitAndPresent(SwapchainHandle p_swapchain, CommandListHandle p_command_list) -> bool;

	// Uploads signal this timeline; graphics submissions wait on its latest value.
	[[nodiscard]] auto TST_GPU_API getTransferTimelineSemaphore() -> SemaphoreHandle;
	[[nodiscard]] auto TST_GPU_API acquireTransferTimelineCounterValue() -> uint64; // returns the current value, then increments the counter
	[[nodiscard]] auto TST_GPU_API getTransferTimelineCounterValue() -> uint64;

	auto TST_GPU_API defferBufferDeletion(BufferHandle p_buffer) -> void;
	auto TST_GPU_API defferTextureDeletion(TextureHandle p_texture) -> void;
}
