#pragma once

#include "memory/ref_ptr.hpp"
#include "core_typedefs.hpp"
#include "string/string.hpp"

#include <nvrhi/nvrhi.h>

namespace tst::gpu
{
	// A wrapper around a command buffer
	// For Vulkan, this is essentially the same as a (VkCommandBuffer / vk::CommandBuffer), just at a higher level
	class CommandBuffer : public RefCounted
	{
	public:
		// 0 for count means that one will be created per frame in flight.
		// See renderer/renderer_config_info.hpp for more
		explicit CommandBuffer(uint32 count = 0, String debug_name = "Command Buffer");

		void beginRecording();
		void endRecording();
		void submit();

		void _beginRecording();
		void _endRecording();
		void _submit();

		static void lockGraphicsQueue();
		static void unlockGraphicsQueue();

		nvrhi::CommandListHandle getActiveCommandBuffer() const { return m_activeCommandBuffer; }

	private:
		String m_debugName;

		nvrhi::CommandListHandle                          m_activeCommandBuffer;
		nvrhi::static_vector<nvrhi::CommandListHandle, 3> m_commandLists;
	};
}
