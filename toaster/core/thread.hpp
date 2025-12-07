#pragma once

#include <thread>

#include "string.hpp"

namespace tst
{
	class Thread
	{
	public:
		explicit Thread(const String &name);

		template<typename TFunc, typename... Args>
		void dispatch(TFunc &&func, Args &&... args)
		{
			m_thread = std::thread(func, std::forward<Args>(args)...);
			setName(m_name);
		}

		void setName(const String &name);

		void join();

		std::thread::id getID() const;

	private:
		String      m_name;
		std::thread m_thread;
	};
}
