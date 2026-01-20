#pragma once

#ifdef WIN32
#if GL_HPP_BUILD_DLL
#ifdef GL_HPP_DLL_EXPORT
#define TST_API __declspec(dllexport)
#else
#define TST_API __declspec(dllimport)
#endif
#else
#define TST_API
#endif
#endif
