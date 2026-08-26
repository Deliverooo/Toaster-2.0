#pragma once

#include "system_types.hpp"
#include "toast_os.hpp"

namespace toaster::os
{
	// Creates a console and redirects all input/output to it
	TST_OS_API auto createOutputConsole(cstr p_title = "Toaster Debug Output") -> void;

	TST_OS_API auto destroyOutputConsole() -> void;
}
