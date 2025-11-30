#include "render_command_queue.hpp"

#include "core/memory/allocator.hpp"

namespace tst
{
	RenderCommandQueue::RenderCommandQueue()
	{
		m_commandBuffer    = tnew uint8_t[10 * 1024 * 1024]; // 10 MB buffer
		m_commandBufferPtr = m_commandBuffer;
		std::memset(m_commandBuffer, 0, 10 * 1024 * 1024);
	}

	RenderCommandQueue::~RenderCommandQueue()
	{
		delete[] m_commandBuffer;
	}

	void *RenderCommandQueue::alloc(RenderCommandFn func, uint32 size)
	{
		*reinterpret_cast<RenderCommandFn *>(m_commandBufferPtr) = func;
		m_commandBufferPtr += sizeof(RenderCommandFn);

		*reinterpret_cast<uint32_t *>(m_commandBufferPtr) = size;
		m_commandBufferPtr += sizeof(uint32_t);

		void *memory = m_commandBufferPtr;
		m_commandBufferPtr += size;

		m_commandCount++;
		return memory;
	}

	void RenderCommandQueue::executeCommands()
	{
		uint8_t *buffer = m_commandBuffer;

		for (uint32_t i = 0; i < m_commandCount; i++)
		{
			RenderCommandFn function = *reinterpret_cast<RenderCommandFn *>(buffer);
			buffer += sizeof(RenderCommandFn);

			uint32_t size = *reinterpret_cast<uint32_t *>(buffer);
			buffer += sizeof(uint32_t);
			function(buffer);
			buffer += size;
		}

		m_commandBufferPtr = m_commandBuffer;
		m_commandCount     = 0;
	}
}
