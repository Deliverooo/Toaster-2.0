#pragma once

#include <ranges>
#include <vector>

#include "handle.hpp"
#include "toast_assert.h"

namespace toaster
{
	// The best class ever... seriously
	template<typename Tag, typename TData>
	struct Pool
	{
		Pool()  = default;
		~Pool() = default;

		Pool(const Pool &)            = delete;
		Pool(Pool &&)                 = delete;
		Pool &operator=(const Pool &) = delete;
		Pool &operator=(Pool &&)      = delete;

		explicit Pool(uint32 p_reserved_size)
		{
			reserve(p_reserved_size);
		}

		auto reserve(uint32 p_reserved_size) -> void
		{
			m_data.reserve(p_reserved_size);
			m_magic.reserve(p_reserved_size);
			m_alive.reserve(p_reserved_size);
			m_refCounts.reserve(p_reserved_size);
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
				m_refCounts.emplace_back(0u);
			}

			m_alive[index]     = true;
			m_refCounts[index] = 0u;

			return Handle<Tag>{index, m_magic[index]};
		}

		auto create(const TData &p_object) -> Handle<Tag>
		{
			uint32 index{0u};
			if (!m_freeIndices.empty())
			{
				index = m_freeIndices.back();
				m_freeIndices.pop_back();
			}
			else
			{
				index = static_cast<uint32>(m_data.size());
				m_data.emplace_back();
				m_alive.emplace_back();
				m_magic.emplace_back(1u);
				m_refCounts.emplace_back(0u);
			}

			m_alive[index]     = true;
			m_data[index]      = p_object;
			m_refCounts[index] = 0u;

			return Handle<Tag>{index, m_magic[index]};
		}

		auto create(TData &&p_data) -> Handle<Tag>
		{
			return emplace(std::move(p_data));
		}

		// The return value should only be used to perform destruction logic on the data and is not meant to be stored
		auto destroy(Handle<Tag> p_handle) -> TData *
		{
			#ifndef TST_DISABLE_POOL_VALIDATION
			if (!isValid(p_handle))
				TST_PERMA_ASSERT_MSG(false, "Your handle is not valid... :(");
			#endif

			m_alive[p_handle.id] = false;
			++m_magic[p_handle.id];
			m_refCounts[p_handle.id] = 0u;

			m_freeIndices.push_back(p_handle.id);

			return &m_data[p_handle.id];
		}

		auto isValid(Handle<Tag> p_handle) const -> bool
		{
			if (p_handle.id >= m_data.size())
				return false;

			return m_alive[p_handle.id] && (m_magic[p_handle.id] == p_handle.magic);
		}

		auto incRef(Handle<Tag> p_handle) -> void
		{
			#ifndef TST_DISABLE_POOL_VALIDATION
			if (isValid(p_handle))
				++m_refCounts[p_handle.id];
			else
				TST_PERMA_ASSERT_MSG(false, "Handle is not valid");
			#else
			++m_refCounts[p_handle.id];
			#endif
		}

		auto decRef(Handle<Tag> p_handle) -> TData *
		{
			if (isValid(p_handle))
			{
				if (m_refCounts[p_handle.id] == 0u)
					return nullptr;
				if ((--m_refCounts[p_handle.id]) == 0u)
					return destroy(p_handle);
			}

			return nullptr;
		}

		auto getData(Handle<Tag> p_handle) -> TData *
		{
			#ifndef TST_DISABLE_POOL_VALIDATION
			if (!isValid(p_handle))
				return nullptr;
			#endif
			return std::addressof(m_data[p_handle.id]);
		}

		auto getData(Handle<Tag> p_handle) const -> const TData *
		{
			#ifndef TST_DISABLE_POOL_VALIDATION
			if (!isValid(p_handle))
				return nullptr;
			#endif
			return std::addressof(m_data[p_handle.id]);
		}

		auto getSize() const -> uint32 { return m_data.size(); }
		auto getCapacity() const -> uint32 { return m_data.capacity(); }

		// Only use for explicit lookup from a specific index.
		auto dataAt(uint32 p_index) -> TData & { return m_data[p_index]; }
		auto isAliveAt(uint32 p_index) const -> bool { return m_alive[p_index]; }

		template<typename TFunc>
		auto forEachAlive(TFunc &&p_func) -> void
		{
			for (uint32 i{0u}; i < m_data.size(); ++i)
				if (m_alive[i])
					p_func(m_data[i]);
		}

	private:
		std::vector<TData>  m_data;
		std::vector<uint8>  m_alive;
		std::vector<uint32> m_magic;
		std::vector<uint32> m_freeIndices;
		std::vector<uint32> m_refCounts;
	};
}
