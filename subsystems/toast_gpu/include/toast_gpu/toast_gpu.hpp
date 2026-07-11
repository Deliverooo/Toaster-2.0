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
	class VKGPUContext;
}

#define TST_GPU_OBJECT\
	private:\
		::toaster::NonOwningPtr<::toaster::gpu::VKGPUContext> m_gpuCtx{nullptr};\
	public:\
		[[nodiscard]] auto getGPUCtx() const -> ::toaster::NonOwningPtr<::toaster::gpu::VKGPUContext> { return m_gpuCtx; }\
	private:

#define TST_GPU_DEFINE_HANDLE(__type, __name) using __name##Handle = ::toaster::RefPtr<__type>; using __name = __type; using __name##Unique = ::toaster::UniquePtr<__type>;
