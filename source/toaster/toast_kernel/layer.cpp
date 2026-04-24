#include "layer.hpp"

#include "application.hpp"
#include "window.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster
{
	IAppLayer::~IAppLayer()
	{
		// m_appParent->getWindow().getLogicalDevice()->performGarbageCollection();
	}
}
