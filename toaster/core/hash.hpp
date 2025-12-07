#pragma once
#include "core_typedefs.hpp"
#include "core/string.hpp"

namespace tst::hash
{
	inline uint32 fnvHash(const StringView str)
	{
		static constexpr uint32 FNV_PRIME    = 0x01000193;
		static constexpr uint32 OFFSET_BASIS = 0x811c9dc5;

		const uint64 length = str.length();
		const char * data   = str.data();

		uint32 hash = OFFSET_BASIS;
		for (uint64 i = 0; i < length; ++i)
		{
			hash ^= *data++;
			hash *= FNV_PRIME;
		}
		hash ^= '\0';
		hash *= FNV_PRIME;

		return hash;
	}
}
