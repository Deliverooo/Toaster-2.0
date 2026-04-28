#pragma once

#ifdef TST_BUILD_DLL
#ifdef TST_DLL_EXPORT
#define TST_API __declspec(dllexport)
#else
#define TST_API __declspec(dllimport)
#endif
#else
#define TST_API
#endif

#define TST_STATIC_INTERFACE(__name) class __declspec(novtable) TST_API __name final
