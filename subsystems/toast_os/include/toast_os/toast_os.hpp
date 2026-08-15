#pragma once

#ifdef TST_OS_BUILD_DLL
#ifdef TST_OS_DLL_EXPORT
#define TST_OS_API __declspec(dllexport)
#else
#define TST_OS_API __declspec(dllimport)
#endif
#else
#define TST_OS_API
#endif

#include <Windows.h>
#undef min
#undef max
