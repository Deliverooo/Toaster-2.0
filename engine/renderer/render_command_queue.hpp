#pragma once

#include "core/core_typedefs.hpp"

namespace tst
{
	class RenderCommandQueue
	{
	public:
		using RenderCommandFn = void (*)(void *);

		RenderCommandQueue();
		~RenderCommandQueue();

		void *alloc(RenderCommandFn func, uint32 size);

		void executeCommands();

	private:
		uint8 *m_commandBuffer;
		uint8 *m_commandBufferPtr;
		uint32 m_commandCount = 0;
	};
}
