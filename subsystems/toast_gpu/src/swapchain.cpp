#include "toast_gpu/swapchain.hpp"

namespace toaster::gpu
{
	Swapchain::Swapchain(LogicalDevice &p_logical_device, vk::SurfaceKHR p_surface) : m_windowSurface(p_surface)
	{
	}

	auto Swapchain::beginFrame() -> void
	{
	}

	auto Swapchain::endFrame() -> void
	{
	}

	auto Swapchain::presentFrame() -> void
	{
	}

	auto Swapchain::_createSwapchain() -> void
	{
	}

	auto Swapchain::_createSwapchainObjects() -> void
	{
	}

	auto Swapchain::_recreateSwapchain() -> void
	{
	}
}
