#include "thread.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace tst
{
	Thread::Thread(const std::string &name) : m_name(name)
	{
	}

	void Thread::setName(const std::string &name)
	{
		#ifdef _WIN32

		HANDLE threadHandle = m_thread.native_handle();

		std::wstring wName(name.begin(), name.end());
		SetThreadDescription(threadHandle, wName.c_str());
		SetThreadAffinityMask(threadHandle, 8);

		#else

		#endif
	}

	void Thread::join()
	{
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

	std::thread::id Thread::getID() const
	{
		return m_thread.get_id();
	}
}
