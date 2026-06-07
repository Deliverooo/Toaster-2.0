#pragma once

#ifdef TST_GPU_BUILD_DLL
#ifdef TST_GPU_DLL_EXPORT
#define TST_GPU_API __declspec(dllexport)
#else
#define TST_GPU_API __declspec(dllimport)
#endif
#else
#define TST_GPU_API
#endif

#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKInstance;
	class VKPhysicalDevice;
	class VKLogicalDevice;
}

#define TST_GPU_OBJECT\
	private:\
		::toaster::NonOwningPtr<::toaster::gpu::VKLogicalDevice> m_device{nullptr};\
	public:\
		[[nodiscard]] auto getDevice() const -> ::toaster::NonOwningPtr<::toaster::gpu::VKLogicalDevice> { return m_device; }\
	private:

#define TST_GPU_DEFINE_HANDLE(__type, __name) using __name##Handle = ::toaster::RefPtr<__type>; using __name = __type; using __name##Unique = ::toaster::UniquePtr<__type>;
