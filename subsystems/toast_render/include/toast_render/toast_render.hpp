#pragma once

#ifdef TST_RENDER_BUILD_DLL
#ifdef TST_RENDER_DLL_EXPORT
#define TST_RENDER_API __declspec(dllexport)
#else
#define TST_RENDER_API __declspec(dllimport)
#endif
#else
#define TST_RENDER_API
#endif

#include <toast_lib/core_basic.hpp>
