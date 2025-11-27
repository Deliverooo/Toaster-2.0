#include "gpu/rendering_context.hpp"

namespace tst::gpu
{
	WindowSurfaceID RenderingContext::getWindowSurface(WindowID window_id)
	{
		auto it = m_windowSurfaceMap.find(window_id);
		if (it != m_windowSurfaceMap.end())
		{
			return it->second;
		}
		return WindowSurfaceID{};
	}

	EError RenderingContext::createWindow(WindowID window_id, const void *platform_data)
	{
		WindowSurfaceID surface_id = createSurface(platform_data);
		if (surface_id != INVALID_WINDOW_SURFACE)
		{
			m_windowSurfaceMap[window_id] = surface_id;
			return EError::eOk;
		}
		return EError::eFailedToCreate;
	}

	void RenderingContext::destroyWindow(WindowID window_id)
	{
		WindowSurfaceID surface_id = getWindowSurface(window_id);
		if (surface_id != INVALID_WINDOW_SURFACE)
		{
			destroySurface(surface_id);
		}
		m_windowSurfaceMap.erase(window_id);
	}

	EVsyncMode RenderingContext::getVsyncMode(WindowID window_id)
	{
		WindowSurfaceID surface_id = getWindowSurface(window_id);
		if (surface_id != INVALID_WINDOW_SURFACE)
		{
			return getSurfaceVsyncMode(surface_id);
		}
		return EVsyncMode::eDisabled;
	}

	void RenderingContext::setVsyncMode(WindowID window_id, EVsyncMode vsync_mode)
	{
		WindowSurfaceID surface_id = getWindowSurface(window_id);
		if (surface_id != INVALID_WINDOW_SURFACE)
		{
			return setSurfaceVsyncMode(surface_id, vsync_mode);
		}
	}

	void RenderingContext::setWindowSize(WindowID window_id, uint32 width, uint32 height)
	{
		WindowSurfaceID surface_id = getWindowSurface(window_id);
		if (surface_id != INVALID_WINDOW_SURFACE)
		{
			setSurfaceSize(surface_id, width, height);
		}
	}
}
