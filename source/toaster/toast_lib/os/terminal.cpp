#include "terminal.hpp"

#include <Windows.h>

namespace toaster::os
{
	auto getExecutablePath() -> io::filesystem::Path
	{
		char path[MAX_PATH];
		GetModuleFileNameA(nullptr, path, MAX_PATH);
		return path;
	}

	auto getBinaryDirectory() -> io::filesystem::Path
	{
		return getExecutablePath().parent_path();
	}
}
