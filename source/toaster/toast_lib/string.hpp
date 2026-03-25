#pragma once

#include <string>

namespace toaster
{
	using String    = std::string;
	using WString   = std::wstring;
	using U8String  = std::u8string;
	using U16String = std::u16string;
	using U32String = std::u32string;

	_NODISCARD inline U8String to_u8string(int _Val)
	{
		return std::_Integral_to_string<char8_t>(_Val);
	}

	_NODISCARD inline U8String to_u8string(unsigned int _Val)
	{
		return std::_UIntegral_to_string<char8_t>(_Val);
	}

	_NODISCARD inline U8String to_u8string(long _Val)
	{
		return std::_Integral_to_string<char8_t>(_Val);
	}

	_NODISCARD inline U8String to_u8string(unsigned long _Val)
	{
		return std::_UIntegral_to_string<char8_t>(_Val);
	}

	_NODISCARD inline U8String to_u8string(long long _Val)
	{
		return std::_Integral_to_string<char8_t>(_Val);
	}

	_NODISCARD inline U8String to_u8string(unsigned long long _Val)
	{
		return std::_UIntegral_to_string<char8_t>(_Val);
	}

	_NODISCARD inline U8String to_u8string(double _Val)
	{
		const auto _Len = static_cast<size_t>(_CSTD _scprintf("%f", _Val));
		U8String   _Str(_Len, '\0');
		_CSTD sprintf_s(reinterpret_cast<char *>(&_Str[0]), _Len + 1, "%f", _Val);
		return _Str;
	}

	_NODISCARD inline U8String to_u8string(float _Val)
	{
		return to_u8string(static_cast<double>(_Val));
	}

	_NODISCARD inline U8String to_u8string(long double _Val)
	{
		return to_u8string(static_cast<double>(_Val));
	}


}
