#pragma once

#include "render_command_queue.hpp"
#include "core/core.hpp"
#include "platform/render_thread.hpp"

namespace tst
{
	// Static renderer interface
	class Renderer
	{
	public:
		template<typename FuncT>
		static void submit(FuncT &&func)
		{
			auto renderCmd = [](void *ptr)
			{
				auto pFunc = static_cast<FuncT *>(ptr);
				(*pFunc)();

				pFunc->~FuncT();
			};
			auto storageBuffer = _getRenderCommandQueue().alloc(renderCmd, sizeof(func));
			new(storageBuffer) FuncT(std::forward<FuncT>(func));
		}

		static void waitAndRender(RenderThread *renderThread);
		static void swapQueues();

		static void   renderThreadFunc(RenderThread *renderThread);
		static uint32 getRenderQueueIndex();
		static uint32 getRenderQueueSubmissionIndex();

		static void beginFrame();
		static void endFrame();

		static uint32_t getCurrentFrameIndex();  // From the application
		static uint32_t _getCurrentFrameIndex(); // From the window's swapchain

		// Initialization
		static EError init();
		static void   terminate();

	private:
		static RenderCommandQueue &_getRenderCommandQueue();
	};
}
