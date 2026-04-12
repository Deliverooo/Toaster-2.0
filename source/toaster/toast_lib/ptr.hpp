#pragma once

#include <atomic> // I am atomic
#include <functional>
#include <memory>

namespace toaster
{
	// Literally a better std::shared_ptr
	template<typename Type>
	class RefPtr
	{
	public:
		using Deleter = std::function<void(Type *)>;

		RefPtr(std::nullptr_t) : m_ptr(nullptr), m_controlBlock(nullptr)
		{
		}

		RefPtr(Type *p_ptr = nullptr, Deleter p_deleter = [](Type *p) { delete p; }) : m_ptr(p_ptr),
																					   m_controlBlock(p_ptr ? new ControlBlock(1, std::move(p_deleter)) : nullptr)
		{
		}

		RefPtr(const RefPtr &p_other) : m_ptr(p_other.m_ptr), m_controlBlock(p_other.m_controlBlock)
		{
			_incRef();
		}

		template<typename TOther>
		RefPtr(const RefPtr<TOther> &p_other) : m_ptr(dynamic_cast<Type *>(p_other.m_ptr)), m_controlBlock(reinterpret_cast<ControlBlock *>(p_other.m_controlBlock))
		{
			_incRef();
		}

		template<typename TOther>
		RefPtr(RefPtr<TOther> &&p_other) : m_ptr(dynamic_cast<Type *>(p_other.m_ptr)), m_controlBlock(reinterpret_cast<ControlBlock *>(p_other.m_controlBlock))
		{
			p_other.m_ptr          = nullptr;
			p_other.m_controlBlock = nullptr;
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
				m_ptr          = p_other.m_ptr;
				m_controlBlock = p_other.m_controlBlock;
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
				m_ptr          = p_other.m_ptr;
				m_controlBlock = p_other.m_controlBlock;
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
				m_ptr                  = p_other.m_ptr;
				m_controlBlock         = p_other.m_controlBlock;
				p_other.m_ptr          = nullptr;
				p_other.m_controlBlock = nullptr;
			}
			return *this;
		}

		Type &operator*() { return *m_ptr; }
		Type *operator->() { return m_ptr; }

		Type &operator*() const { return *m_ptr; }
		Type *operator->() const { return m_ptr; }

		Type *      get() { return m_ptr; }
		const Type *get() const { return m_ptr; }

		void reset(Type *p_ptr = nullptr, Deleter p_deleter = [](Type *p) { delete p; })
		{
			if (m_ptr == p_ptr)
				return;
			_release();
			m_ptr          = p_ptr;
			m_controlBlock = p_ptr ? new ControlBlock(1, std::move(p_deleter)) : nullptr;
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
			if (m_controlBlock)
				++(m_controlBlock->refCount);
		}

		void _release()
		{
			if (m_controlBlock)
			{
				if (--(m_controlBlock->refCount) == 0)
				{
					m_controlBlock->deleter(m_ptr);
					delete m_controlBlock;
				}
				m_ptr          = nullptr;
				m_controlBlock = nullptr;
			}
		}

		struct ControlBlock
		{
			std::atomic_int32_t refCount;
			Deleter             deleter;

			ControlBlock(int32_t p_count, Deleter p_deleter) : refCount(p_count), deleter(std::move(p_deleter))
			{
			}
		};

		mutable Type *        m_ptr{nullptr};
		mutable ControlBlock *m_controlBlock{nullptr};

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

	#if 0
	template<typename Type>
	class UniquePtr
	{
	public:
		UniquePtr ~UniquePtr()
		{
			delete m_ptr;
		}

	private:
		Type *m_ptr{nullptr};
	};

	#endif

	template<typename Type>
	using UniquePtr = std::unique_ptr<Type>;

	template<typename Type, typename... TArgs>
	RefPtr<Type> make_reference(TArgs &&... p_args)
	{
		return RefPtr<Type>(new Type(std::forward<TArgs>(p_args)...));
	}

	template<typename Type, typename... TArgs>
	RefPtr<Type> allocate_reference(typename RefPtr<Type>::Deleter &&p_deleter, TArgs &&... p_args)
	{
		return RefPtr<Type>(new Type(std::forward<TArgs>(p_args)...), std::move(p_deleter));
	}

	template<typename Type, typename... TArgs>
	UniquePtr<Type> make_unique(TArgs &&... p_args)
	{
		return std::make_unique<Type>(std::forward<TArgs>(p_args)...);
	}
}
