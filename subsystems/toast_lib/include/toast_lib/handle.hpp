#pragma once

#include "system_types.h"

namespace toaster
{
	template<typename Tag>
	struct Handle
	{
		constexpr Handle() = default;

		constexpr Handle(uint32 p_id, uint32 p_magic) : id(p_id), magic(p_magic)
		{
		}

		uint32 id   : 20;
		uint32 magic: 12;

		constexpr auto valid() const -> bool { return magic != 0u; }
	};
}

#define TST_DECLARE_HANDLE(__tag) \
	struct __tag##Tag;\
	using __tag##Handle = ::toaster::Handle<__tag##Tag>
