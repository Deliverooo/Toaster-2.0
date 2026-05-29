#pragma once

#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"

#include <Windows.h>

#include <libloaderapi.h>

namespace toaster::os
{
	#ifdef WIN32
	using LibraryHandle = HMODULE;
	#else
	#error "Too bad!!"
	#endif

	TST_LIB_API auto loadLibrary(CString p_library_name) -> LibraryHandle;
	TST_LIB_API auto loadLibrary(CWString p_library_name) -> LibraryHandle;

	template<typename TFunc>
	auto getProcAddress(const LibraryHandle p_library, const CString p_func_name) -> TFunc
	{
		return reinterpret_cast<TFunc>(GetProcAddress(p_library, p_func_name));
	}
}
