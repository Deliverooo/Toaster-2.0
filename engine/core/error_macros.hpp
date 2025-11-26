#pragma once

#include "core_defines.hpp"

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include <cstdio>

#ifdef _MSC_VER
#define DEBUG_BREAK() __debugbreak()

#define GENERATE_TRAP() __fastfail(7)

#else
#define DEBUG_BREAK() attribute(__debugbreak__)
#define GENERATE_TRAP() __builtin_trap()
#endif

#define TST_ASSERT(cond)\
	if (UNLIKELY(!(cond)))\
	{\
		printf("[%s][%s][%d], CRITICAL: Assertion failed: \"" _STR(cond) "\"",__FILE__, __FUNCTION__, __LINE__);\
		fflush(stdout);\
		GENERATE_TRAP();\
	}
