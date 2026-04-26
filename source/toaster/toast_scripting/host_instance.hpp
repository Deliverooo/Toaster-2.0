#pragma once

#include "../toaster_export.hpp"

#include <hostfxr.h>

#include "toast_lib/io/filesystem.hpp"
#include "toast_lib/os/library_loading.hpp"

namespace toaster
{
	struct TST_API HostInstanceSpecInfo
	{
		io::filesystem::Path configPath{};
		io::filesystem::Path assemblyPath{};
	};

	class TST_API HostInstance
	{
	public:
		HostInstance(const HostInstanceSpecInfo &p_spec_info);
		~HostInstance();

	private:
		HostInstanceSpecInfo m_specInfo{};

		os::LibraryHandle m_hostfxrLibraryHandle{nullptr};
		hostfxr_handle    m_hostfxrContext{nullptr};
	};
}
