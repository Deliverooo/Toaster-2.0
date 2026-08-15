#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"

#include <Windows.h>

namespace toaster
{
	WString convertUtf8ToWide(const String &p_string)
	{
		int  length{MultiByteToWideChar(CP_UTF8, 0, p_string.data(), static_cast<int>(p_string.length()), nullptr, 0)};
		auto result{WString((uint64) length, wchar_t{0})};
		MultiByteToWideChar(CP_UTF8, 0, p_string.data(), static_cast<int>(p_string.length()), result.data(), length);
		return result;
	}

	String convertWideToUtf8(const WString &p_string)
	{
		int    requiredLength{WideCharToMultiByte(CP_UTF8, 0, p_string.data(), static_cast<int>(p_string.length()), nullptr, 0, nullptr, nullptr)};
		String result((uint64) requiredLength, 0);
		(void) WideCharToMultiByte(CP_UTF8, 0, p_string.data(), static_cast<int>(p_string.length()), result.data(), requiredLength, nullptr, nullptr);
		return result;
	}
}
