#include "host_instance.hpp"

#include <coreclr_delegates.h>
#include <nethost.h>

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

namespace toaster
{
	HostInstance::HostInstance(const HostInstanceSpecInfo &p_spec_info) : m_specInfo(p_spec_info)
	{
		using hello_fn = int(*)(void *args, int sizeBytes);

		char_t buffer[MAX_PATH];
		size_t buffer_size{sizeof(buffer) / sizeof(char_t)};
		int    rc{get_hostfxr_path(buffer, &buffer_size, nullptr)};
		TST_ASSERT(rc == 0);

		m_hostfxrLibraryHandle = os::loadLibrary(buffer);
		m_initFn               = toaster::os::getProcAddress<hostfxr_initialize_for_runtime_config_fn>(m_hostfxrLibraryHandle, "hostfxr_initialize_for_runtime_config");
		m_closeFn              = toaster::os::getProcAddress<hostfxr_close_fn>(m_hostfxrLibraryHandle, "hostfxr_close");

		m_initFn(m_specInfo.configPath.wstring().c_str(), nullptr, &m_hostfxrContext);

		void *load_assembly_and_get_function_pointer = nullptr;
		auto  get_delegate_fptr{toaster::os::getProcAddress<hostfxr_get_runtime_delegate_fn>(m_hostfxrLibraryHandle, "hostfxr_get_runtime_delegate")};
		get_delegate_fptr(m_hostfxrContext, hdt_load_assembly_and_get_function_pointer, &load_assembly_and_get_function_pointer);

		auto load_assembly_ptr = (load_assembly_and_get_function_pointer_fn) load_assembly_and_get_function_pointer;

		// 3. Load the C# DLL and get the function pointer
		hello_fn hello = nullptr;
		load_assembly_ptr(m_specInfo.assemblyPath.wstring().c_str(), L"Toaster.Test, TestManagedLibrary", // Namespace.Class, Assembly
						  L"hello",                                                                       // Method name
						  nullptr,                                                                        // Default delegate type
						  nullptr, (void **) &hello);

		// 4. Call the C# method
		if (hello)
		{
			int result = hello(nullptr, 0);
			LOG_INFO("{}", result);
		}
	}

	HostInstance::~HostInstance()
	{
		m_closeFn(m_hostfxrContext);
	}
}
