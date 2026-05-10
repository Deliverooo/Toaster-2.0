#pragma once
#include "toaster_macros.hpp"

#include "../io/filesystem.hpp"

namespace toaster::os
{
	TST_API auto getExecutablePath() -> io::filesystem::Path;
	TST_API auto getBinaryDirectory() -> io::filesystem::Path;
}
