#pragma once

#ifdef TST_UI_BUILD_DLL
#ifdef TST_UI_DLL_EXPORT
#define TST_UI_API __declspec(dllexport)
#else
#define TST_UI_API __declspec(dllimport)
#endif
#else
#define TST_UI_API
#endif

namespace breadcrumb
{
}
