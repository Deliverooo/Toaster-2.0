#include "platform/window_manager.hpp"

#ifdef TST_PLATFORM_WINDOWS
#include "platform/windows/windows_window_manager.hpp"
#endif
namespace tst
{
	static WindowManager *singleton = nullptr;

	WindowManager::WindowManager()
	{
		singleton = this;
	}

	WindowManager *WindowManager::create()
	{
		#ifdef TST_PLATFORM_WINDOWS
		return new WindowsWindowManager();
		#endif
	}

	WindowManager *WindowManager::get()
	{
		return singleton;
	}
}
