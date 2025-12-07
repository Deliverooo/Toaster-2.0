#pragma once

#include "ref_ptr.hpp"
#include "core_typedefs.hpp"
#include "string.hpp"

#include <nvrhi/nvrhi.h>
#include <vulkan/vulkan.hpp>

namespace tst
{
	// A wrapper around a command buffer
	// For Vulkan, this is essentially the same as a (VkCommandBuffer / vk::CommandBuffer), just at a higher level
	class CommandBuffer : public RefCounted
	{
	public:
		// 0 for count means that one will be created per frame in flight.
		// See renderer/renderer_config_info.hpp for more
		explicit CommandBuffer(uint32 count = 0, String debug_name = "Command Buffer", bool owned_by_swapchain = false);

		void beginRecording();
		void endRecording();
		void submit();

		void _beginRecording();
		void _endRecording();
		void _submit();

		void _wait(vk::Semaphore waitSemaphore);

		nvrhi::GraphicsState &      getGraphicsState() { return m_graphicsState; }
		const nvrhi::GraphicsState &getGraphicsState() const { return m_graphicsState; }
		void                        setGraphicsState(nvrhi::GraphicsState &graphicsState) { m_graphicsState = graphicsState; }
		void                        _commitGraphicsState();

		nvrhi::ComputeState &      getComputeState() { return m_computeState; }
		const nvrhi::ComputeState &getComputeState() const { return m_computeState; }
		void                       setComputeState(nvrhi::ComputeState &computeState) { m_computeState = computeState; }
		void                       _commitComputeState();

		static void lockGraphicsQueue();
		static void unlockGraphicsQueue();

		nvrhi::CommandListHandle getActiveCommandBuffer() const { return m_activeCommandBuffer; }

	private:
		String m_debugName;

		nvrhi::GraphicsState m_graphicsState;
		nvrhi::ComputeState  m_computeState;

		nvrhi::CommandListHandle                          m_activeCommandBuffer;
		nvrhi::static_vector<nvrhi::CommandListHandle, 3> m_commandLists;
	};
}
