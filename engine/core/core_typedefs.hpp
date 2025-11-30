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
#include <type_traits>

namespace tst
{
	#if defined(_WIN64)

	using int8  = __int8;
	using int16 = __int16;
	using int32 = __int32;
	using int64 = __int64;

	using uint8  = unsigned __int8;
	using uint16 = unsigned __int16;
	using uint32 = unsigned __int32;
	using uint64 = unsigned __int64;

	#else

	using int8 = char; using int16 = short; using int32 = int; using int64 = long long; using uint8 = unsigned char; using uint16 = unsigned short; using uint32 =
	unsigned int; using uint64 = unsigned long long;

	#endif

	using float32 = float;
	using float64 = double;

	template<typename Type> concept real_c = std::is_floating_point_v<Type>;
	template<typename Type> concept integer_c = std::is_integral_v<Type>;
}
