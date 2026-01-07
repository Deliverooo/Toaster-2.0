#pragma once

#include <exception>
#include <string>
#include <typeinfo>

namespace toaster
{
	class Exception : public std::exception
	{
	public:
		Exception(int p_line, const char *file) noexcept : m_line(p_line), m_file(file)
		{
		}

		[[nodiscard]] virtual const char *what() const noexcept override;

		virtual const char *getType() const noexcept
		{
			return typeid(decltype(*this)).name();
		}

		int         getLine() const noexcept { return m_line; }
		const char *getFile() const noexcept { return m_file; }

	protected:
		int         m_line;
		const char *m_file;

		mutable std::string m_what;
	};
}
