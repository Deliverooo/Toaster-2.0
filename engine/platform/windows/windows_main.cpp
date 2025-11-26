#include <Windows.h>
#include <print>

#include "platform/window_manager.hpp"
#include "main/main.hpp"
#include "platform/windows/windows_os.hpp"

namespace tst
{
	int _main(int argc, char **argv)
	{
		WindowsOS os{GetModuleHandle(nullptr)};

		if (const EError err = Main::init(); err != EError::eOk)
		{
			return EXIT_FAILURE;
		}

		if (Main::start() == EError::eOk)
		{
			while (true)
			{
				WindowManager::get()->processEvents();
				if (!Main::loop())
				{
					break;
				}
			}
		}

		Main::cleanup();

		return 0;
	}
}

#ifndef TST_DIST

int main(int argc, char **argv)
{
	return tst::_main(__argc, __argv);
}

#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	return tst::_main(__argc, __argv);
}
#endif
