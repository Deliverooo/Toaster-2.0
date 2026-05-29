#pragma once

#ifdef TST_SCRIPT_BUILD_DLL
#ifdef TST_SCRIPT_DLL_EXPORT
#define TST_SCRIPT_API __declspec(dllexport)
#else
#define TST_SCRIPT_API __declspec(dllimport)
#endif
#else
#define TST_SCRIPT_API
#endif

// Everyone should get the core stuff
#include "toast_lib/core_basic.hpp"
