#pragma once

#ifdef TST_SCENE_BUILD_DLL
#ifdef TST_SCENE_DLL_EXPORT
#define TST_SCENE_API __declspec(dllexport)
#else
#define TST_SCENE_API __declspec(dllimport)
#endif
#else
#define TST_SCENE_API
#endif

// Everyone should get the core stuff
#include "toast_lib/core_basic.hpp"
