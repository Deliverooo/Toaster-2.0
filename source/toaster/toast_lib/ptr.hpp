#pragma once

#include <memory>

namespace toaster
{
	template<typename Type>
	class RefPtr
	{
	public:
		RefPtr(std::nullptr_t) : m_ptr(nullptr), m_refCount(nullptr)
		{
		}

		RefPtr(Type *p_ptr = nullptr) : m_ptr(p_ptr), m_refCount(p_ptr ? new std::atomic_int32_t(1) : nullptr)
		{
		}

		RefPtr(const RefPtr &p_other) : m_ptr(p_other.m_ptr), m_refCount(p_other.m_refCount)
		{
			_incRef();
		}

		template<typename TOther>
		RefPtr(const RefPtr<TOther> &p_other) : m_ptr(dynamic_cast<Type *>(p_other.m_ptr)), m_refCount(p_other.m_refCount)
		{
			_incRef();
		}

		template<typename TOther>
		RefPtr(RefPtr<TOther> &&p_other) : m_ptr(dynamic_cast<Type *>(p_other.m_ptr)), m_refCount(p_other.m_refCount)
		{
			p_other.m_ptr      = nullptr;
			p_other.m_refCount = nullptr;
		}

		~RefPtr() { _release(); }

		RefPtr &operator=(std::nullptr_t)
		{
			_release();
			m_ptr = nullptr;
			return *this;
		}

		RefPtr &operator=(const RefPtr &p_other)
		{
			if (this != &p_other)
			{
				_release();
				m_ptr      = p_other.m_ptr;
				m_refCount = p_other.m_refCount;
				_incRef();
			}
			return *this;
		}

		template<typename TOther>
		RefPtr &operator=(const RefPtr<TOther> &p_other)
		{
			if (this != &p_other)
			{
				_release();
				m_ptr      = p_other.m_ptr;
				m_refCount = p_other.m_refCount;
				_incRef();
			}
			return *this;
		}

		template<typename TOther>
		RefPtr &operator=(RefPtr<TOther> &&p_other) noexcept
		{
			if (this != &p_other)
			{
				_release();
				m_ptr              = p_other.m_ptr;
				m_refCount         = p_other.m_refCount;
				p_other.m_ptr      = nullptr;
				p_other.m_refCount = nullptr;
			}
			return *this;
		}

		Type &operator*() { return *m_ptr; }
		Type *operator->() { return m_ptr; }

		Type &operator*() const { return *m_ptr; }
		Type *operator->() const { return m_ptr; }

		Type *      get() { return m_ptr; }
		const Type *get() const { return m_ptr; }

		void reset(Type *p_ptr = nullptr)
		{
			if (m_ptr == p_ptr)
				return;
			_release();
			m_ptr      = p_ptr;
			m_refCount = p_ptr ? new std::atomic_int32_t(1) : nullptr;
		}

		template<typename TOther>
		RefPtr<TOther> as() const
		{
			return RefPtr<TOther>(*this);
		}

		bool operator==(const RefPtr &p_other) const
		{
			return m_ptr == p_other.m_ptr;
		}

		bool operator!=(const RefPtr &p_other) const
		{
			return m_ptr != p_other.m_ptr;
		}

		operator bool() { return m_ptr != nullptr; }
		operator bool() const { return m_ptr != nullptr; }

	private:
		void _incRef()
		{
			if (m_refCount)
				++(*m_refCount);
		}

		void _release()
		{
			if (m_refCount)
			{
				if (--(*m_refCount) == 0)
				{
					delete m_ptr;
					delete m_refCount;
				}
				m_ptr      = nullptr;
				m_refCount = nullptr;
			}
		}

		mutable Type *               m_ptr{nullptr};
		mutable std::atomic_int32_t *m_refCount{nullptr};

		template<typename TOther>
		friend class RefPtr;
	};

	template<typename Type>
	class WeakRefPtr
	{
	public:
		WeakRefPtr() = default;

		WeakRefPtr(const RefPtr<Type> &p_ref_ptr)
		{
			m_ptr = p_ref_ptr.get();
		}

		WeakRefPtr(Type *p_ptr)
		{
			m_ptr = p_ptr;
		}

		Type &operator*() { return *m_ptr; }
		Type *operator->() { return m_ptr; }

		Type &operator*() const { return *m_ptr; }
		Type *operator->() const { return m_ptr; }

		operator bool() const { return false; } // TODO: ts

		template<typename TOther>
		WeakRefPtr<TOther> as() const
		{
			return WeakRefPtr<TOther>(dynamic_cast<TOther *>(m_ptr));
		}

	private:
		Type *m_ptr{nullptr};
	};

	template<typename Type>
	using UniquePtr = std::unique_ptr<Type>;

	template<typename Type, typename... TArgs>
	RefPtr<Type> make_reference(TArgs &&... p_args)
	{
		return RefPtr<Type>(new Type(std::forward<TArgs>(p_args)...));
	}

	template<typename Type, typename... TArgs>
	UniquePtr<Type> make_unique(TArgs &&... p_args)
	{
		return std::make_unique<Type>(std::forward<TArgs>(p_args)...);
	}
}
