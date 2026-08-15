#pragma once

#include "system_types.hpp"

#define TST_ANSI_WINMAIN() int WINAPI WinMain(::toaster::os::Instance p_instance, ::toaster::os::Instance, ::toaster::os::str p_cmd_line, int p_show_cmd)
#define TST_UNICODE_WINMAIN() int WINAPI wWinMain(::toaster::os::Instance p_instance, ::toaster::os::Instance, ::toaster::os::wstr p_cmd_line, int p_show_cmd)

#define TST_WINMAIN() TST_UNICODE_WINMAIN()
