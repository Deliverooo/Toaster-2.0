#pragma once

#ifdef TST_RENDER_BUILD_DLL
#ifdef TST_RENDER_DLL_EXPORT
#define TST_RENDER_API __declspec(dllexport)
#else
#define TST_RENDER_API __declspec(dllimport)
#endif
#else
#define TST_RENDER_API
#endif

// Everyone should get the core stuff
#include "toast_lib/core_basic.hpp"

#define TST_RENDER_DEFINE_HANDLE(__type, __name) using __name##Handle = ::toaster::RefPtr<__type>; using __name##Unique = ::toaster::UniquePtr<__type>;
#define TST_RENDER_OBJECT\
						private:\
						::toaster::NonOwningPtr<::toaster::render::RenderContext> m_renderCtx{nullptr};\
						public:\
						[[nodiscard]] auto getRenderCtx() const -> ::toaster::NonOwningPtr<::toaster::render::RenderContext> { return m_renderCtx; }\
						private:
