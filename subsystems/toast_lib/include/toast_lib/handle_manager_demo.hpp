#pragma once

#include <vector>
#include "system_types.h"

// Basically, while I was looking for ways to optimise and improve my engine architecture, I kept seeing people use handle-based systems alongside their ecs.
// I then discovered that what I was doing previously (spamming shared pointers everywhere) was not good for cache locality and a poor choice with an ecs
// So from here on, I will try to lean more into a Data-Oriented approach to rendering, hopefully utilising GPU-driven rendering. :) :)
namespace toaster::demo
{
	template<typename Tag>
	struct Handle
	{
		uint32 id   : 20;
		uint32 magic: 12;
	};

	#define TST_DEMO_DECLARE_HANDLE(__tag) struct _##__tag##Tag; using __tag##Handle = Handle<_##__tag##Tag>;

	TST_DEMO_DECLARE_HANDLE(Orbo);
	TST_DEMO_DECLARE_HANDLE(Peeb);

	// From what I can see, this is an example of the 'Handle-based lookup pattern'.
	// This is only a demo of the structure of a handle management class could look like
	template<typename THandleTag, typename TEntryData> requires std::is_default_constructible_v<TEntryData>
	struct Manager
	{
		struct Entry
		{
			TEntryData data;

			uint32 magic{0u};
			bool32 alive{false};
		};

		std::vector<Entry>  entries;
		std::vector<uint32> freeIndices;

		template<typename... TArgs>
		auto createData(TArgs &&... p_args) -> Handle<THandleTag>
		{
			uint32 index{0u};
			if (!freeIndices.empty())
			{
				index = freeIndices.back();
				freeIndices.pop_back();
			}
			else
			{
				index = static_cast<uint32>(entries.size());
				entries.emplace_back();
			}

			entries[index].data  = TEntryData{std::forward<TArgs>(p_args)...};
			entries[index].alive = true;
			++entries[index].magic;

			return Handle<THandleTag>{index, entries[index].magic};
		}

		auto destroyData(Handle<THandleTag> p_handle) -> void
		{
			if (!isValid(p_handle))
				return;

			entries[p_handle.id].alive = false;
			freeIndices.push_back(p_handle.id);
		}

		auto isValid(Handle<THandleTag> p_handle) -> bool
		{
			if (p_handle.id >= entries.size())
				return false;
			return entries[p_handle.id].alive && (entries[p_handle.id].magic == p_handle.magic);
		}

		auto getData(Handle<THandleTag> p_handle) -> TEntryData *
		{
			if (!isValid(p_handle))
				return nullptr;

			return &entries[p_handle.id].data;
		}
	};
}
