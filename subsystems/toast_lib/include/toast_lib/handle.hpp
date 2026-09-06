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

	// Better because it is 64 bit and more like a pointer
	template<typename Type>
	struct Handle2
	{
		uint64 id   : 32;
		uint64 magic: 32;

		constexpr Handle2() : id(0u), magic(0u)
		{
		}

		constexpr Handle2(uint32 p_id, uint32 p_magic) : id(p_id), magic(p_magic)
		{
		}

		constexpr Handle2(nulltype) : id(0u), magic(0u) // This makes it look better
		{
		}

		constexpr Handle2(uint64 p_handle) : id(p_handle & 0xFFFFFFFF), magic((p_handle >> 32) & 0x7FFFFFFF)
		{
		}

		[[nodiscard]] constexpr auto valid() const -> bool { return magic != 0u; } // Only checks if the handle itself is valid, not that the pool thinks it is

		auto operator<=>(const Handle2 &) const = default; // I need to be able to run algorithms on ts

		constexpr          operator bool() const { return valid(); }
		explicit constexpr operator uint64() const { return id | (magic << 32); }


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


template<typename Tag>
struct std::hash<toaster::Handle2<Tag> >
{
	auto operator()(const toaster::Handle2<Tag> &p_handle) const noexcept -> std::size_t
	{
		return hash<uint32>{}(p_handle.id) ^ hash<uint32>{}(p_handle.magic);
	}
};
