#include "library_loading.hpp"

namespace toaster::os
{
	auto loadLibrary(CString p_library_name) -> LibraryHandle
	{
		return LoadLibraryA(p_library_name);
	}

	auto loadLibrary(CWString p_library_name) -> LibraryHandle
	{
		return LoadLibraryW(p_library_name);
	}
}
