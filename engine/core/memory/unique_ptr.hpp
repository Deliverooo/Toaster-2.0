#pragma once

namespace tst
{
	template<typename Type>
	class UniquePtr
	{
	public:
		UniquePtr(Type *p_ptr) { m_ptr = p_ptr; }

		~UniquePtr();
		UniquePtr(UniquePtr &&)            = delete;
		UniquePtr &operator=(UniquePtr &&) = delete;

	private:
		Type *m_ptr;
	};
}
