#pragma once

namespace gl
{
	using Enum     = unsigned int;
	using Bool     = bool;
	using Bitfield = unsigned int;
	using Void     = void;
	using Byte     = signed char;
	using UByte    = unsigned char;
	using Short    = short;
	using UShort   = unsigned short;
	using Int      = int;
	using UInt     = unsigned int;
	using Int64    = long long;
	using UInt64   = unsigned long long;
	using SizeI    = int;
	using Float    = float;
	using Double   = double;
	using Char     = char;
	using UChar    = unsigned char;

	using IntPtr   = __int64;
	using UIntPtr  = unsigned __int64;
	using SizeIPtr = __int64;

	using DebugCallbackProc = void (__stdcall *)(Enum p_source, Enum p_type, UInt p_id, Enum p_severity, SizeI p_length, const Char *p_message, const void *p_user_param);

	using ID = UInt; // Useful for gl objects (Framebuffers, vaos, vbos...)

	using CString = const char *;
}
