#pragma once

#include <ranges>
#include <utility>
#include "initialiser_list.hpp"
#include "system_types.h"
#include "toast_assert.h"

namespace toaster
{
	template<typename TKey, typename TVal, uint64 Size>
	class MapConstexpr
	{
	public:
		using Entry = std::pair<TKey, TVal>;

		constexpr MapConstexpr(const std::initializer_list<Entry> &p_entries)
		{
			TST_ASSERT_MSG(p_entries.size() <= Size, "Out of range");
			std::copy(p_entries.begin(), p_entries.end(), m_map.begin());
		}

		template<typename... TArgs>
		constexpr MapConstexpr(TArgs &&... p_entries) : m_map{std::forward<TArgs>(p_entries)...}
		{
			static_assert(sizeof...(TArgs) <= Size, "Out of range");
		}

		constexpr auto at(const TKey &p_key) const -> const TVal &
		{
			const auto it{
				std::ranges::find_if(m_map, [&](const Entry &p_entry) -> bool
				{
					return p_entry.first == p_key;
				})
			};
			if (it != m_map.end())
				return it->second;
			TST_ASSERT_MSG(false, "Invalid key provided");
			return m_map.front().second;
		}

	private:
		std::array<Entry, Size> m_map;
	};
}
