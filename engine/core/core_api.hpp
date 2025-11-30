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

// The Core API shared library export macro definitions

#ifdef TST_PLATFORM_WINDOWS
#ifdef TST_BUILD_SHARED_LIBS
#ifdef TST_CORE_EXPORT
#define TST_CORE_API __declspec(dllexport)
#else
#define TST_CORE_API __declspec(dllimport)
#endif
#else
#define TST_CORE_API
#endif
#else
#define TST_CORE_API
#endif