#include "main/main.hpp"

#include <print>

#include "../platform/window_manager.hpp"
#include "platform/os.hpp"

namespace tst
{
	static WindowManager *s_windowManager = nullptr;

	EError Main::init()
	{
		s_windowManager = WindowManager::create();

		return EError::eOk;
	}

	EError Main::start()
	{
		std::println("Hello World!");

		return EError::eOk;
	}

	bool Main::loop()
	{
		return true;
	}

	void Main::cleanup()
	{
	}
}
