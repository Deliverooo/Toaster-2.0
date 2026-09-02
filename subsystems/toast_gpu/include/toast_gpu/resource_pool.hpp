#pragma once

#include "toast_lib/handle.hpp"
#include "toast_lib/toast_assert.h"

#include <ranges>
#include <vector>

namespace toaster::gpu
{
	template<typename Tag, typename TData>
	class ResourcePool
	{
	public:
		ResourcePool()  = default;
		~ResourcePool() = default;

		ResourcePool(const ResourcePool &)            = delete;
		ResourcePool(ResourcePool &&)                 = delete;
		ResourcePool &operator=(const ResourcePool &) = delete;
		ResourcePool &operator=(ResourcePool &&)      = delete;

		explicit ResourcePool(uint32 p_reserved_size)
		{
			reserve(p_reserved_size);
		}

		auto reserve(uint32 p_reserved_size) -> void
		{
			m_data.reserve(p_reserved_size);
			m_magic.reserve(p_reserved_size);
			m_alive.reserve(p_reserved_size);
		}

		template<typename... TArgs>
		auto emplace(TArgs &&... p_args) -> Handle<Tag>
		{
			uint32 index{0u};
			if (!m_freeIndices.empty())
			{
				index = m_freeIndices.back();
				m_freeIndices.pop_back();

				if constexpr (std::is_move_assignable_v<TData>)
					m_data[index] = TData{std::forward<TArgs>(p_args)...};
				else
				{
					m_data[index].~TData();
					::new(std::addressof(m_data[index])) TData{std::forward<TArgs>(p_args)...};
				}
			}
			else
			{
				index = static_cast<uint32>(m_data.size());
				m_data.emplace_back(std::forward<TArgs>(p_args)...);
				m_alive.emplace_back();
				m_magic.emplace_back(1u);
			}

			m_alive[index] = true;

			return Handle<Tag>{index, m_magic[index]};
		}

		auto destroy(Handle<Tag> p_handle, uint64 p_target_timeline_value) -> void
		{
			if (!isValid(p_handle))
				TST_PERMA_ASSERT_MSG(false, "Your handle is not valid... :(");

			m_alive[p_handle.id] = false;
			++m_magic[p_handle.id];

			m_deferredDeletions.emplace_back(DeferredDeletion{p_handle.id, p_target_timeline_value});
		}

		template<typename TFunc>
		auto cleanupDeletions(uint64 p_current_timeline_value, TFunc &&p_func) -> void
		{
			for (uint64 i{0u}; i < m_deferredDeletions.size();)
			{
				auto &entry{m_deferredDeletions[i]};
				if (p_current_timeline_value >= entry.targetTimelineValue)
				{
					p_func(m_data[entry.resourceId]);

					m_freeIndices.push_back(entry.resourceId);

					m_deferredDeletions[i] = m_deferredDeletions.back();
					m_deferredDeletions.pop_back();
				}
				else
					++i;
			}
		}

		template<typename TFunc>
		auto purgeAll(TFunc &&p_func) -> void
		{
			for (auto &deletion: m_deferredDeletions)
				p_func(m_data[deletion.resourceId]);
			m_deferredDeletions.clear();

			forEachAlive(std::forward<TFunc>(p_func));
			m_data.clear();
			m_alive.clear();
			m_magic.clear();
			m_freeIndices.clear();
		}

		template<typename TFunc>
		auto forEachAlive(TFunc &&p_func) -> void
		{
			for (uint64 i{0u}; i < m_data.size(); ++i)
				if (m_alive[i])
					p_func(m_data[i]);
		}

		auto isValid(Handle<Tag> p_handle) const -> bool
		{
			if (p_handle.id >= m_data.size())
				return false;

			return m_alive[p_handle.id] && (m_magic[p_handle.id] == p_handle.magic);
		}

		auto getData(Handle<Tag> p_handle) -> TData *
		{
			if (!isValid(p_handle))
				return nullptr;
			return std::addressof(m_data[p_handle.id]);
		}

		auto getData(Handle<Tag> p_handle) const -> const TData *
		{
			if (!isValid(p_handle))
				return nullptr;
			return std::addressof(m_data[p_handle.id]);
		}

		// Only use for explicit lookup from a specific index.
		auto dataAt(uint32 p_index) -> TData & { return m_data[p_index]; }
		auto isAliveAt(uint32 p_index) const -> bool { return m_alive[p_index]; }

	private:
		std::vector<TData>  m_data;
		std::vector<uint8>  m_alive;
		std::vector<uint32> m_magic;
		std::vector<uint32> m_freeIndices;

		struct DeferredDeletion
		{
			uint32 resourceId;
			uint64 targetTimelineValue;
		};

		std::vector<DeferredDeletion> m_deferredDeletions;
	};
}
