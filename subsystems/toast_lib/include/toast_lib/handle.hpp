#pragma once

#include "system_types.h"

namespace toaster
{
	template<typename Tag>
	struct Handle
	{
		constexpr Handle() : id(0u), magic(0u)
		{
		}

		constexpr Handle(uint32 p_id, uint32 p_magic) : id(p_id), magic(p_magic)
		{
		}

		uint32 id   : 20;
		uint32 magic: 12;

		constexpr auto valid() const -> bool { return magic != 0u; } // Only checks if the handle itself is valid, not that the pool thinks it is

		auto operator<=>(const Handle &) const = default; // I need to be able to run algorithms on ts
	};
}

#define TST_DECLARE_HANDLE(__tag) \
struct __tag##Tag;\
using __tag##Handle = ::toaster::Handle<__tag##Tag>

template<typename Tag>
struct std::hash<toaster::Handle<Tag> >
{
	auto operator()(const toaster::Handle<Tag> &p_handle) const noexcept -> std::size_t
	{
		return hash<uint32>{}(p_handle.id);
	}
};
