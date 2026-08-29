#pragma once

#include <ranges>
#include <vector>

#include "handle.hpp"
#include "ptr.hpp"
#include "toast_assert.h"

namespace toaster
{
	// The best class ever... seriously
	template<typename Tag, typename TData> /*requires std::is_default_constructible_v<TData>*/
	struct Pool
	{
		Pool() = default;
		~Pool() = default;

		Pool(const Pool &) = delete;
		Pool(Pool &&) = delete;
		Pool &operator=(const Pool &) = delete;
		Pool &operator=(Pool &&) = delete;

		explicit Pool(uint32 p_reserved_size)
		{
			reserve(p_reserved_size);
		}

		auto reserve(uint32 p_reserved_size) -> void
		{
			_data.reserve(p_reserved_size);
			m_magic.reserve(p_reserved_size);
			_alive.reserve(p_reserved_size);
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
					_data[index] = TData{std::forward<TArgs>(p_args)...};
				else
				{
					_data[index].~TData();
					::new(std::addressof(_data[index])) TData{std::forward<TArgs>(p_args)...};
				}
			}
			else
			{
				index = static_cast<uint32>(_data.size());
				_data.emplace_back(std::forward<TArgs>(p_args)...);
				_alive.emplace_back();
				m_magic.emplace_back(1u);
				m_refCounts.emplace_back(0u);
			}

			_alive[index] = true;
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
				index = static_cast<uint32>(_data.size());
				_data.emplace_back();
				_alive.emplace_back();
				m_magic.emplace_back(1u);
				m_refCounts.emplace_back(0u);
			}

			_alive[index] = true;
			_data[index] = p_object;
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

			_alive[p_handle.id] = false;
			++m_magic[p_handle.id];
			m_refCounts[p_handle.id] = 0u;

			m_freeIndices.push_back(p_handle.id);

			return &_data[p_handle.id];
		}

		auto isValid(Handle<Tag> p_handle) const -> bool
		{
			if (p_handle.id >= _data.size())
				return false;

			return _alive[p_handle.id] && (m_magic[p_handle.id] == p_handle.magic);
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

		auto decRef(Handle<Tag> p_handle) -> TData*
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
			return std::addressof(_data[p_handle.id]);
		}

		auto getData(Handle<Tag> p_handle) const -> const TData *
		{
#ifndef TST_DISABLE_POOL_VALIDATION
			if (!isValid(p_handle))
				return nullptr;
#endif
			return std::addressof(_data[p_handle.id]);
		}

		auto getSize() const -> uint32 { return _data.size(); }
		auto getCapacity() const -> uint32 { return _data.capacity(); }

		auto getAlive() -> std::vector<TData *>
		{
			std::vector<TData *> alive;
			for (uint32 i{0u}; i < _data.size(); ++i)
			{
				if (_alive[i])
					alive.emplace_back(&_data[i]);
			}
			return alive;
		}

		std::vector<TData> _data;
		// Maybe you will want to access this directly, to prevent accidents, I prefix it with _
		std::vector<bool> _alive;
		// I know this is a vector of bools, but it doesn't matter because I won't be taking the addresses
	private:
		std::vector<uint32> m_magic;
		std::vector<uint32> m_freeIndices;
		std::vector<uint32> m_refCounts;
	};

	// template<typename Tag, typename TData>
	// class SharedHandle
	// {
	// public:
	// 	SharedHandle() : m_pool(nullptr)
	// 	{
	// 	}
	//
	// 	SharedHandle(Handle<Tag> p_handle, Pool<Tag, TData> *p_pool) : m_pool(p_pool), m_handle(p_handle)
	// 	{
	// 		if (m_pool)
	// 			m_pool->incRef(m_handle);
	// 	}
	//
	// 	~SharedHandle()
	// 	{
	// 		if (m_pool)
	// 			m_pool->decRef(m_handle);
	// 	}
	//
	// 	SharedHandle(const SharedHandle &p_other) : m_pool(p_other.m_pool), m_handle(p_other.m_handle)
	// 	{
	// 		if (m_pool)
	// 			m_pool->incRef(m_handle);
	// 	}
	//
	// 	SharedHandle(SharedHandle &&p_other) noexcept : m_pool(p_other.m_pool), m_handle(p_other.m_handle)
	// 	{
	// 		p_other.m_pool = nullptr;
	// 	}
	//
	// 	auto operator=(const SharedHandle &p_other) -> SharedHandle &
	// 	{
	// 		if (this != &p_other)
	// 		{
	// 			if (m_pool)
	// 				m_pool->decRef(m_handle);
	// 			m_handle = p_other.m_handle;
	// 			m_pool = p_other.m_pool;
	// 			if (m_pool)
	// 				m_pool->incRef(m_handle);
	// 		}
	// 		return *this;
	// 	}
	//
	// 	auto operator=(SharedHandle &&p_other) noexcept -> SharedHandle &
	// 	{
	// 		if (this != &p_other)
	// 		{
	// 			if (m_pool)
	// 				m_pool->decRef(m_handle);
	// 			m_handle = p_other.m_handle;
	// 			m_pool = p_other.m_pool;
	//
	// 			p_other.m_handle = {};
	// 			p_other.m_pool = nullptr;
	// 		}
	// 		return *this;
	// 	}
	//
	// 	auto operator*() -> TData & { return *m_pool->getData(m_handle); }
	// 	auto operator->() -> TData * { return m_pool ? m_pool->getData(m_handle) : nullptr; }
	//
	// 	auto operator*() const -> const TData & { return *m_pool->getData(m_handle); }
	// 	auto operator->() const -> const TData * { return m_pool ? m_pool->getData(m_handle) : nullptr; }
	//
	// 	auto get() const -> Handle<Tag> { return m_handle; }
	// 	auto isValid() const -> bool { return m_pool && m_pool->isValid(m_handle); }
	//
	// 	operator bool() const { return isValid(); }
	//
	// private:
	// 	NonOwningPtr<Pool<Tag, TData> > m_pool{nullptr};
	// 	Handle<Tag> m_handle{};
	// };
}
