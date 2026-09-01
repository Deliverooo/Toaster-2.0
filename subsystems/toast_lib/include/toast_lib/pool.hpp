#pragma once

#include <ranges>
#include <vector>

#include "handle.hpp"
#include "toast_assert.h"

namespace toaster
{
	// The best class ever... seriously
	template<typename Tag, typename TData> /*requires std::is_default_constructible_v<TData>*/
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

			_alive[index]      = true;
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

			_alive[index]      = true;
			_data[index]       = p_object;
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
		std::vector<uint8> _alive;
		// I know this is a vector of bools, but it doesn't matter because I won't be taking the addresses
	private:
		std::vector<uint32> m_magic;
		std::vector<uint32> m_freeIndices;
		std::vector<uint32> m_refCounts;
	};

	// template<typename TManager, typename THandleType> requires requires(TManager p_manager, THandleType p_handle)
	// {
	// 	p_manager.acquire(p_handle); p_manager.release(p_handle); { p_manager.isValid(p_handle) } -> std::same_as<bool>;
	// }
	template<typename TManager, typename THandleType>
	class Ref
	{
	public:
		Ref() noexcept = default;

		Ref(TManager *p_manager, THandleType p_handle) noexcept
			: m_manager(p_manager), m_handle(p_handle)
		{
		}

		~Ref()
		{
			reset();
		}

		Ref(const Ref &p_other) noexcept : m_manager(p_other.m_manager), m_handle(p_other.m_handle)
		{
			if (m_manager && m_handle.valid())
				m_manager->acquire(m_handle);
		}

		Ref(Ref &&p_other) noexcept : m_manager(p_other.m_manager), m_handle(p_other.m_handle)
		{
			p_other.m_manager = nullptr;
			p_other.m_handle  = {};
		}

		Ref &operator=(const Ref &p_other) noexcept
		{
			if (this != &p_other)
			{
				reset();

				m_manager = p_other.m_manager;
				m_handle  = p_other.m_handle;

				if (m_manager && m_handle.valid())
					m_manager->acquire(m_handle);
			}
			return *this;
		}

		Ref &operator=(Ref &&p_other) noexcept
		{
			if (this != &p_other)
			{
				reset();

				m_manager = p_other.m_manager;
				m_handle  = p_other.m_handle;

				p_other.m_manager = nullptr;
				p_other.m_handle  = {};
			}
			return *this;
		}

		void reset() noexcept
		{
			if (m_manager && m_handle.valid())
				m_manager->release(m_handle);

			m_manager = nullptr;
			m_handle  = {};
		}

		[[nodiscard]] THandleType get() const noexcept
		{
			return m_handle;
		}

		[[nodiscard]] bool valid() const noexcept
		{
			return m_manager && m_handle.valid() && m_manager->isValid(m_handle);
		}

		[[nodiscard]] auto operator->() const -> auto
		{
			return m_manager->getData(m_handle);
		}

		[[nodiscard]] auto operator->() -> auto
		{
			return m_manager->getData(m_handle);
		}

		[[nodiscard]] auto manager() -> TManager *
		{
			return m_manager;
		}

		[[nodiscard]] auto manager() const -> const TManager *
		{
			return m_manager;
		}

		explicit operator bool() const noexcept
		{
			return valid();
		}

		THandleType operator*() const noexcept
		{
			return m_handle;
		}

	private:
		TManager *  m_manager{nullptr};
		THandleType m_handle{};
	};
}
