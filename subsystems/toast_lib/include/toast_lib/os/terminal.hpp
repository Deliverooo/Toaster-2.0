#pragma once

#include "toast_lib/toast_lib.hpp"

#include "toast_lib/io/filesystem.hpp"

namespace toaster::os
{
	TST_LIB_API auto getExecutablePath() -> io::filesystem::Path;
	TST_LIB_API auto getBinaryDirectory() -> io::filesystem::Path;
}
