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

	template<typename Tag, typename TData>
	class Ref;

	template<typename Tag, typename TData>
	class ResourceManager
	{
	public:
		using HandleType = Handle<Tag>;
		using RefType    = Ref<Tag, TData>;
		using Deleter    = void(*)(void *, TData *);

		ResourceManager() = default;

		ResourceManager(Deleter p_deleter, void *p_deleter_user_data) : m_deleter(p_deleter), m_deleterUserData(p_deleter_user_data)
		{
			TST_ASSERT(m_deleter);
		}

		auto setDeleter(Deleter p_deleter) { m_deleter = p_deleter; }
		auto setDeleterUserData(void *p_deleter_user_data) { m_deleterUserData = p_deleter_user_data; }

		template<typename... TArgs>
		[[nodiscard]] auto create(TArgs &&... p_args) -> RefType
		{
			HandleType handle{m_pool.emplace(std::forward<TArgs>(p_args)...)};
			m_pool.incRef(handle);
			return RefType{this, handle};
		}

		auto acquire(HandleType p_handle) -> void { m_pool.incRef(p_handle); }

		auto release(HandleType p_handle) -> void
		{
			if (TData *data{m_pool.decRef(p_handle)})
				m_deleter(m_deleterUserData, data);
		}

		auto isValid(HandleType p_handle) const -> bool { return m_pool.isValid(p_handle); }
		auto getData(HandleType p_handle) const -> const TData * { return m_pool.getData(p_handle); }
		auto getData(HandleType p_handle) -> TData * { return m_pool.getData(p_handle); }

		template<typename TFunc>
		auto forEachAlive(TFunc &&p_func) -> void
		{
			m_pool.forEachAlive(std::forward<TFunc>(p_func));
		}

	private:
		Pool<Tag, TData> m_pool;

		Deleter m_deleter{nullptr};
		void *  m_deleterUserData{nullptr};
	};

	template<typename Tag, typename TData>
	class Ref
	{
	public:
		using ManagerType = ResourceManager<Tag, TData>;
		using HandleType  = Handle<Tag>;

		Ref() noexcept = default;

		Ref(ManagerType *p_manager, HandleType p_handle) noexcept : m_manager(p_manager), m_handle(p_handle)
		{
		}

		~Ref() { reset(); }

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

		[[nodiscard]] HandleType get() const noexcept { return m_handle; }

		[[nodiscard]] bool valid() const noexcept { return m_manager && m_handle.valid() && m_manager->isValid(m_handle); }

		[[nodiscard]] auto operator->() const -> auto { return m_manager->getData(m_handle); }
		[[nodiscard]] auto operator->() -> auto { return m_manager->getData(m_handle); }

		[[nodiscard]] auto manager() -> ManagerType * { return m_manager; }
		[[nodiscard]] auto manager() const -> const ManagerType * { return m_manager; }

		explicit operator bool() const noexcept { return valid(); }

		HandleType operator*() const noexcept { return m_handle; }

	private:
		ManagerType *m_manager{nullptr};
		HandleType   m_handle{};
	};

	#define TST_DECLARE_REF(__tag) \
	using __tag##Ref = ::toaster::Ref<__tag##Tag, __tag##Data>
}
