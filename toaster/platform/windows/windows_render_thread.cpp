#include "platform/render_thread.hpp"
#include "renderer/renderer.hpp"

#include <Windows.h>

namespace tst
{
	struct RenderThreadData
	{
		CRITICAL_SECTION   m_criticalSection{};
		CONDITION_VARIABLE m_conditionVariable{};

		RenderThread::EState m_state = RenderThread::EState::eIdle;
	};

	static std::thread::id s_renderThreadID;

	RenderThread::RenderThread(EThreadingPolicy threading_policy) : m_threadingPolicy(threading_policy), m_renderThread("Render Thread")
	{
		m_data = new RenderThreadData();

		if (m_threadingPolicy == EThreadingPolicy::eMultiThreaded)
		{
			InitializeCriticalSection(&m_data->m_criticalSection);
			InitializeConditionVariable(&m_data->m_conditionVariable);
		}
	}

	RenderThread::~RenderThread()
	{
		if (m_threadingPolicy == EThreadingPolicy::eMultiThreaded)
		{
			DeleteCriticalSection(&m_data->m_criticalSection);
		}

		s_renderThreadID = std::thread::id();
	}

	void RenderThread::run()
	{
		m_isRunning = true;
		if (m_threadingPolicy == EThreadingPolicy::eMultiThreaded)
		{
			m_renderThread.dispatch(Renderer::renderThreadFunc, this);
		}

		s_renderThreadID = m_renderThread.getID();
	}

	void RenderThread::terminate()
	{
		m_isRunning = false;
		pump();

		if (m_threadingPolicy == EThreadingPolicy::eMultiThreaded)
		{
			m_renderThread.join();
		}

		s_renderThreadID = std::thread::id();
	}

	void RenderThread::wait(EState wait_for_state)
	{
		if (m_threadingPolicy == EThreadingPolicy::eSingleThreaded)
		{
			return;
		}

		EnterCriticalSection(&m_data->m_criticalSection);
		while (m_data->m_state != wait_for_state)
		{
			// This releases the CS so that another thread can wake it
			SleepConditionVariableCS(&m_data->m_conditionVariable, &m_data->m_criticalSection, INFINITE);
		}
		LeaveCriticalSection(&m_data->m_criticalSection);
	}

	void RenderThread::waitAndSet(EState wait_for_state, EState set_to_state)
	{
		if (m_threadingPolicy == EThreadingPolicy::eSingleThreaded)
		{
			return;
		}

		EnterCriticalSection(&m_data->m_criticalSection);
		while (m_data->m_state != wait_for_state)
		{
			SleepConditionVariableCS(&m_data->m_conditionVariable, &m_data->m_criticalSection, INFINITE);
		}
		m_data->m_state = set_to_state;
		WakeAllConditionVariable(&m_data->m_conditionVariable);
		LeaveCriticalSection(&m_data->m_criticalSection);
	}

	void RenderThread::set(EState set_to_state)
	{
		if (m_threadingPolicy == EThreadingPolicy::eSingleThreaded)
		{
			return;
		}

		EnterCriticalSection(&m_data->m_criticalSection);
		m_data->m_state = set_to_state;
		WakeAllConditionVariable(&m_data->m_conditionVariable);
		LeaveCriticalSection(&m_data->m_criticalSection);
	}

	void RenderThread::nextFrame()
	{
		m_appThreadFrame++;
		Renderer::swapQueues();
	}

	void RenderThread::blockUntilRenderComplete()
	{
		if (m_threadingPolicy == EThreadingPolicy::eSingleThreaded)
		{
			return;
		}

		wait(EState::eIdle);
	}

	void RenderThread::kick()
	{
		if (m_threadingPolicy == EThreadingPolicy::eMultiThreaded)
		{
			set(EState::eKick);
		}
		else
		{
			Renderer::waitAndRender(this);
		}
	}

	void RenderThread::pump()
	{
		nextFrame();
		kick();
		blockUntilRenderComplete();
	}

	bool RenderThread::isCurrentThreadRenderThread()
	{
		return s_renderThreadID == std::this_thread::get_id();
	}
} // namespace tst
