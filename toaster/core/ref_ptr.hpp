#pragma once

#include "allocator.hpp"
#include "core_typedefs.hpp"
#include "ref_ptr.hpp"

namespace tst
{
	class RefCounted
	{
	public:
		virtual ~RefCounted() = default;

		void incRefCount() const
		{
			++m_refCount;
		}

		void decRefCount() const
		{
			--m_refCount;
		}

		uint32 getRefCount() const { return m_refCount.load(); }

	private:
		mutable std::atomic<uint32> m_refCount{0};
	};

	void _addToLiveReferences(void *instance);
	void _removeFromLiveReferences(void *instance);
	bool _isLive(void *instance);

	template<typename Type>
	class RefPtr
	{
	public:
		RefPtr()
			: m_instance(nullptr)
		{
		}

		RefPtr(std::nullptr_t n)
			: m_instance(nullptr)
		{
		}

		RefPtr(Type *instance)
			: m_instance(instance)
		{
			static_assert(std::is_base_of_v<RefCounted, Type>, "Class is not RefCounted!");

			_incRef();
		}

		template<typename TOther> requires(std::is_base_of_v<TOther, Type> || std::is_base_of_v<Type, TOther>)
		RefPtr(const RefPtr<TOther> &other)
		{
			m_instance = static_cast<Type *>(other.m_instance);
			_incRef();
		}

		template<typename TOther>
		RefPtr(RefPtr<TOther> &&other)
		{
			m_instance       = static_cast<Type *>(other.m_instance);
			other.m_instance = nullptr;
		}

		~RefPtr()
		{
			_decRef();
		}

		RefPtr(const RefPtr &other)
			: m_instance(other.m_instance)
		{
			_incRef();
		}

		RefPtr &operator=(std::nullptr_t)
		{
			_decRef();
			m_instance = nullptr;
			return *this;
		}

		RefPtr &operator=(const RefPtr &other)
		{
			other._incRef();
			_decRef();

			m_instance = other.m_instance;
			return *this;
		}

		template<typename TOther>
		RefPtr &operator=(const RefPtr<TOther> &other)
		{
			other._incRef();
			_decRef();

			m_instance = other.m_instance;
			return *this;
		}

		template<typename TOther>
		RefPtr &operator=(RefPtr<TOther> &&other)
		{
			_decRef();

			m_instance       = other.m_instance;
			other.m_instance = nullptr;
			return *this;
		}

		operator bool() { return m_instance != nullptr; }
		operator bool() const { return m_instance != nullptr; }

		Type *      operator->() { return m_instance; }
		const Type *operator->() const { return m_instance; }

		Type &      operator*() { return *m_instance; }
		const Type &operator*() const { return *m_instance; }

		Type *      get() { return m_instance; }
		const Type *get() const { return m_instance; }

		void reset(Type *instance = nullptr)
		{
			_decRef();
			m_instance = instance;
		}

		template<typename TOther> requires(std::is_base_of_v<TOther, Type> || std::is_base_of_v<Type, TOther>)
		RefPtr<TOther> as() const
		{
			return RefPtr<TOther>(*this);
		}

		bool operator==(const RefPtr<Type> &other) const
		{
			return m_instance == other.m_instance;
		}

		bool operator!=(const RefPtr<Type> &other) const
		{
			return !(*this == other);
		}

	private:
		void _incRef() const
		{
			if (m_instance)
			{
				m_instance->incRefCount();

				_addToLiveReferences(static_cast<void *>(m_instance));
			}
		}

		void _decRef() const
		{
			if (m_instance)
			{
				m_instance->decRefCount();
				if (m_instance->getRefCount() == 0u)
				{
					_removeFromLiveReferences(static_cast<void *>(m_instance));

					delete m_instance;
					m_instance = nullptr;
				}
			}
		}

		template<class TOther>
		friend class RefPtr;
		mutable Type *m_instance;

		template<class TOther>
		friend class CachedPtr;
	};

	template<typename T, typename... Args>
	RefPtr<T> make_reference(Args &&... args)
	{
		return RefPtr<T>(tnew T(std::forward<Args>(args)...));
	}

	template<typename Type>
	class CachedPtr
	{
	public:
		CachedPtr() = default;

		CachedPtr(RefPtr<Type> ref)
		{
			m_instance = ref.get();
		}

		CachedPtr(Type *instance)
		{
			m_instance = instance;
		}

		Type *      operator->() { return m_instance; }
		const Type *operator->() const { return m_instance; }

		Type &      operator*() { return *m_instance; }
		const Type &operator*() const { return *m_instance; }

		bool isValid() const { return m_instance ? _isLive(m_instance) : false; }
		operator bool() const { return isValid(); }

		template<typename TOther>
		CachedPtr<TOther> as() const
		{
			return CachedPtr<TOther>(dynamic_cast<TOther *>(m_instance));
		}

	private:
		Type *m_instance = nullptr;
	};
}
