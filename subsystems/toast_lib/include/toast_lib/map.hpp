#pragma once

#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

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

	template<typename TKey, typename TValue>
	auto getMapKeysAsVector(const std::unordered_map<TKey, TValue> &p_map) -> std::vector<TKey>
	{
		std::vector<TValue> out_keys;
		for (const auto &key: p_map | std::views::keys)
		{
			out_keys.emplace_back(key);
		}
		return out_keys;
	}

	template<typename TKey, typename TValue>
	auto getMapValuesAsVector(const std::unordered_map<TKey, TValue> &p_map) -> std::vector<TValue>
	{
		std::vector<TValue> out_values;
		for (const auto &val: p_map | std::views::values)
		{
			out_values.emplace_back(val);
		}
		return out_values;
	}

	template<typename TKey, typename TValue, typename TReturnType, typename TGetFn>
	auto getMapValuesAsVector(const std::unordered_map<TKey, TValue> &p_map, TGetFn &&p_get_fn) -> std::vector<TReturnType>
	{
		std::vector<TReturnType> out_vars;
		out_vars.reserve(p_map.size());
		std::transform(p_map.begin(), p_map.end(), std::back_inserter(out_vars), [&](const auto &p_pair) { return p_get_fn(p_pair.second); });
		return out_vars;
	}
}
