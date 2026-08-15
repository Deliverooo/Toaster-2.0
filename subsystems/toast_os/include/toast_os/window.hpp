#pragma once

#include "system_types.hpp"

namespace toaster::os
{
	TST_OS_API auto getWindowBackBufferSize(Wnd p_window, uint &p_out_width, uint &p_out_height) -> void;
}
