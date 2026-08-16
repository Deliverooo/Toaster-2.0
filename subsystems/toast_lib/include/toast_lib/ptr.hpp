#pragma once

#include "system_types.h"

#include <atomic> // I am... atomic
#include <memory>

namespace toaster
{
	// Literally a better std::shared_ptr without virtual dispatch in the control block
	// If you only plan on using a single thread, you don't need to use atomic types.
	template<typename Type, typename TRefCountType = std::atomic_uint32_t>
	class RefPtr
	{
	public:
		RefPtr(nulltype) : m_ptr(nullptr), m_controlBlock(nullptr)
		{
		}

		RefPtr(Type *p_ptr = nullptr) : m_ptr(p_ptr), m_controlBlock(p_ptr ? new ControlBlock(1) : nullptr)
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

		auto operator=(std::nullptr_t) -> RefPtr &
		{
			_release();
			m_ptr = nullptr;
			return *this;
		}

		auto operator=(const RefPtr &p_other) -> RefPtr &
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
		auto operator=(const RefPtr<TOther> &p_other) -> RefPtr &
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
		auto operator=(RefPtr<TOther> &&p_other) noexcept -> RefPtr &
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

		auto operator*() -> Type & { return *m_ptr; }
		auto operator->() -> Type * { return m_ptr; }

		auto operator*() const -> Type & { return *m_ptr; }
		auto operator->() const -> Type * { return m_ptr; }

		auto get() -> Type * { return m_ptr; }
		auto get() const -> const Type * { return m_ptr; }

		auto reset(Type *p_ptr = nullptr) -> void
		{
			if (m_ptr == p_ptr)
				return;
			_release();
			m_ptr          = p_ptr;
			m_controlBlock = p_ptr ? new ControlBlock(1) : nullptr;
		}

		template<typename TOther>
		auto as() const -> RefPtr<TOther>
		{
			return RefPtr<TOther>(*this);
		}

		auto getWithoutConst() const -> Type * requires requires { !std::same_as<Type, std::remove_const_t<Type> >; }
		{
			return m_ptr;
		}

		auto operator==(const RefPtr &p_other) const -> bool
		{
			return m_ptr == p_other.m_ptr;
		}

		auto operator!=(const RefPtr &p_other) const -> bool
		{
			return m_ptr != p_other.m_ptr;
		}

		operator bool() { return m_ptr != nullptr; }
		operator bool() const { return m_ptr != nullptr; }

		operator Type *() { return m_ptr; }
		operator const Type *() const { return m_ptr; }

	private:
		auto _incRef() -> void
		{
			if (m_controlBlock)
				++(m_controlBlock->refCount);
		}

		auto _release() -> void
		{
			if (m_controlBlock)
			{
				if (--(m_controlBlock->refCount) == 0)
				{
					delete m_ptr;
					delete m_controlBlock;
				}
				m_ptr          = nullptr;
				m_controlBlock = nullptr;
			}
		}

		struct ControlBlock
		{
			TRefCountType refCount;

			ControlBlock(int32_t p_count) : refCount(p_count)
			{
			}
		};

		mutable Type *        m_ptr{nullptr};
		mutable ControlBlock *m_controlBlock{nullptr};

		template<typename TOther, typename TOtherRefCountType>
		friend class RefPtr;
	};

	template<typename Type, typename... TArgs>
	auto makeReference(TArgs &&... p_args) -> RefPtr<Type>
	{
		return RefPtr<Type>(new Type(std::forward<TArgs>(p_args)...));
	}

	template<typename Type>
	using UniquePtr = std::unique_ptr<Type>;

	template<typename Type, typename... TArgs>
	auto makeUnique(TArgs &&... p_args) -> UniquePtr<Type>
	{
		return std::make_unique<Type>(std::forward<TArgs>(p_args)...);
	}

	template<typename Type>
	using HeapPtr = Type *;

	template<typename Type>
	using OwningPtr = Type *;

	template<typename Type>
	using NonOwningPtr = Type *;
}
