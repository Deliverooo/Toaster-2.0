#include "layer.hpp"

#include "application.hpp"
#include "window.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_render/globals.hpp"

namespace toaster
{
	IAppLayer::~IAppLayer()
	{
	}

	auto IAppLayer::getApp() -> Application &
	{
		return *m_appParent;
	}

	auto IAppLayer::getGlobals() -> Globals &
	{
		return *m_appParent->m_globals;
	}
}
