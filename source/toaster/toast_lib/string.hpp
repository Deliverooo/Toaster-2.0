#pragma once

#include <string>

namespace toaster
{
	using CString = const char *;

	using String    = std::string;
	using WString   = std::wstring;
	using U8String  = std::u8string;
	using U16String = std::u16string;
	using U32String = std::u32string;

	template<typename Type> requires std::is_arithmetic_v<Type>
	String to_string(Type p_val)
	{
		return std::to_string(p_val);
	}

	template<typename Type> requires std::is_arithmetic_v<Type>
	WString to_wstring(Type p_val)
	{
		return std::to_wstring(p_val);
	}

	template<typename Type> requires std::is_arithmetic_v<Type>
	U8String to_u8string(Type p_val)
	{
		std::string ret = std::to_string(p_val);
		return U8String{ret.begin(), ret.end()};
	}

	template<typename Type> requires std::is_arithmetic_v<Type>
	U16String to_u16string(Type p_val)
	{
		std::string ret = std::to_string(p_val);
		return U16String{ret.begin(), ret.end()};
	}

	template<typename Type> requires std::is_arithmetic_v<Type>
	U32String to_u32string(Type p_val)
	{
		std::string ret = std::to_string(p_val);
		return U32String{ret.begin(), ret.end()};
	}
}
