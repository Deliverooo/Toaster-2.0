#pragma once

#include "platform/window_manager.hpp"

namespace tst::gpu
{
	using WindowSurfaceID = uint64;

	class RenderingContext
	{
	public:
		virtual ~RenderingContext();

		WindowSurfaceID getWindowSurface(WindowID window_id); // Get the id of a window surface from a window

		EError createWindow(WindowID window_id, const void *platform_data);
		// Creates the window surface for a window using the provided platform data. E.g. a HWND and a HINSTANCE
		EVsyncMode getVsyncMode(WindowID window_id);                  // Get the current Vsync mode of a window
		void       setVsyncMode(WindowID window_id, EVsyncMode mode); // Sets the Vsync mode of the specified window

		void setWindowSize(WindowID window_id, uint32 width, uint32 height);

		virtual EError initialize() = 0;

		virtual WindowSurfaceID createSurface(const void *platform_data) = 0;
		virtual void            setSurfaceVsyncMode(WindowSurfaceID surface_id, EVsyncMode vsync_mode) = 0;
		virtual EVsyncMode      getSurfaceVsyncMode(WindowSurfaceID surface_id) = 0;
		virtual void            setSurfaceSize(WindowSurfaceID surface_id, uint32 width, uint32 height) = 0;
		virtual void            getSurfaceWidth(WindowSurfaceID surface_id) = 0;
		virtual void            getSurfaceHeight(WindowSurfaceID surface_id) = 0;
		virtual void            destroySurface(WindowSurfaceID surface_id) = 0;

	private:
		std::unordered_map<WindowID, WindowSurfaceID> m_windowSurfaceMap; // Maps a window to a corresponding API-specific surface;
	};
}
