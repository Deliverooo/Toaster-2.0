#pragma once

#include "core/core_typedefs.hpp"

#include <thread>
#include <atomic>

#include "thread.hpp"

namespace tst
{
	struct RenderThreadData; // Uses Pimpl

	enum class EThreadingPolicy
	{
		// MultiThreaded will create a Render Thread
		eNone, eSingleThreaded, eMultiThreaded
	};

	class RenderThread
	{
	public:
		enum class EState
		{
			eIdle, eBusy, eKick
		};

		RenderThread(EThreadingPolicy threading_policy);
		~RenderThread();

		void run();
		bool isRunning() const { return m_isRunning; }
		void terminate();

		void wait(EState wait_for_state);
		void waitAndSet(EState wait_for_state, EState set_to_state);
		void set(EState set_to_state);

		void nextFrame();
		void blockUntilRenderComplete();
		void kick();

		void pump();

		static bool isCurrentThreadRenderThread();

	private:
		RenderThreadData *m_data;
		EThreadingPolicy  m_threadingPolicy;

		Thread m_renderThread;

		bool m_isRunning = false;

		std::atomic<uint32> m_appThreadFrame = 0;
	};
} // namespace tst
