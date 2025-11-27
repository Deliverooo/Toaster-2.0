#pragma once

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

	template<typename T> concept real_c = std::is_floating_point_v<T>;
}
