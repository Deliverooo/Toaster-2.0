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

	template<typename TData>
	class Pool2
	{
	public:
		using DestructorFn = void(*)(TData *);
		using HandleType   = Handle2<TData>;

		Pool2() = default;

		explicit Pool2(DestructorFn p_destructor_fn) : m_destructorFn(p_destructor_fn)
		{
		}

		auto setDestructorFn(DestructorFn p_destructor_fn) { m_destructorFn = p_destructor_fn; }

		template<typename... TArgs>
		auto emplace(TArgs &&... p_args) -> HandleType
		{
			uint32 id{0u};
			uint32 magic{1u};

			if (!m_freeIndices.empty())
			{
				id = m_freeIndices.back();
				m_freeIndices.pop_back();
				magic = m_entries[id].magic + 1;
			}
			else
			{
				id = m_entries.size();
				m_entries.resize(id + 1u);
			}

			m_entries[id].data  = TData{std::forward<TArgs>(p_args)...};
			m_entries[id].magic = magic;
			m_entries[id].alive = true;

			return HandleType{id, magic};
		}

		auto destroy(HandleType p_handle) -> void
		{
			if (!isValid(p_handle))
				TST_PERMA_ASSERT(false);
				// return;

			m_entries[p_handle.id].alive = false;
			++m_entries[p_handle.id].magic;
			m_freeIndices.push_back(p_handle.id);

			if (m_destructorFn)
				m_destructorFn(std::addressof(m_entries[p_handle.id].data));
		}

		auto clear() -> void
		{
			if (m_destructorFn)
			{
				for (auto &entry: m_entries)
					if (entry.alive)
						m_destructorFn(&entry.data);
			}
			m_entries.clear();
			m_freeIndices.clear();
		}

		auto tryGet(HandleType p_handle) -> TData *
		{
			if (!isValid(p_handle))
				return nullptr;
			return std::addressof(m_entries[p_handle.id].data);
		}

		auto tryGet(HandleType p_handle) const -> const TData *
		{
			if (!isValid(p_handle))
				return nullptr;
			return std::addressof(m_entries[p_handle.id].data);
		}

		auto operator[](HandleType p_handle) -> TData & { return m_entries[p_handle.id].data; }
		auto operator[](HandleType p_handle) const -> const TData & { return m_entries[p_handle.id].data; }

		auto isValid(HandleType p_handle) -> bool
		{
			if (p_handle.id >= m_entries.size())
				return false;

			return m_entries[p_handle.id].alive && (m_entries[p_handle.id].magic == p_handle.magic);
		}

	private:
		struct Entry
		{
			TData  data;
			uint32 magic{0u};
			bool   alive{false};
		};

		std::vector<Entry>  m_entries;
		std::vector<uint32> m_freeIndices;

		DestructorFn m_destructorFn{nullptr};
	};
}
