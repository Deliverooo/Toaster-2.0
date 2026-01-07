#include "toast_exception.hpp"

#include "fmt/format.h"

namespace toaster
{
	const char *Exception::what() const noexcept
	{
		m_what = fmt::format("{} [File] {} [Line] {}", getType(), m_file, m_line);
		return m_what.c_str();
	}
}
