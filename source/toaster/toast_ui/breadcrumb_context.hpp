#pragma once

#include "toast_ui.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace breadcrumb
{
	struct TST_UI_API ContextSpecInfo
	{
	};

	class TST_UI_API Context
	{
	public:
		Context(toaster::gpu::VKLogicalDevice *p_device);

	private:
		toaster::gpu::VKLogicalDevice *m_device{nullptr};

		ContextSpecInfo m_specInfo{};
	};
}
