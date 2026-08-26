#include "toast_os/console.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace toaster::os
{
	auto createOutputConsole(cstr p_title) -> void
	{
		if (AllocConsole())
		{
			std::FILE *fp{nullptr};
			freopen_s(&fp, "CONOUT$", "w", stdout);
			freopen_s(&fp, "CONOUT$", "w", stderr);
			freopen_s(&fp, "CONIN$", "r", stdin);

			std::cout.clear();
			std::clog.clear();
			std::cerr.clear();
			std::cin.clear();

			std::ios_base::sync_with_stdio();
		}
		else
		{
			assert(false && "Failed to create output console");
			std::abort();
		}
		SetConsoleTitle(p_title);
	}

	auto destroyOutputConsole() -> void
	{
		FreeConsole();
	}
}
