/*
*  	Copyright 2025 Orbo Stetson
 *  	Licensed under the Apache License, Version 2.0 (the "License");
 *  	you may not use this file except in compliance with the License.
 *  	You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */
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
