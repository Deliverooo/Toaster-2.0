#pragma once

#include "device.hpp"
#include "platform/window_manager.hpp"

namespace tst::gpu
{
	using WindowSurfaceID                            = uint64;
	constexpr WindowSurfaceID INVALID_WINDOW_SURFACE = 0;

	class RenderingContext
	{
	public:
		virtual ~RenderingContext() = default;

		WindowSurfaceID getWindowSurface(WindowID window_id); // Get the id of a window surface from a window

		EError createWindow(WindowID window_id, const void *platform_data);
		void   destroyWindow(WindowID window_id);
		// Creates the window surface for a window using the provided platform data. E.g. a HWND and a HINSTANCE
		EVsyncMode getVsyncMode(WindowID window_id);                        // Get the current Vsync mode of a window
		void       setVsyncMode(WindowID window_id, EVsyncMode vsync_mode); // Sets the Vsync mode of the specified window

		void setWindowSize(WindowID window_id, uint32 width, uint32 height);

		virtual EError initialize() = 0;

		virtual WindowSurfaceID createSurface(const void *platform_data) = 0;
		virtual void            setSurfaceVsyncMode(WindowSurfaceID surface_id, EVsyncMode vsync_mode) = 0;
		virtual EVsyncMode      getSurfaceVsyncMode(WindowSurfaceID surface_id) = 0;
		virtual void            setSurfaceSize(WindowSurfaceID surface_id, uint32 width, uint32 height) = 0;
		virtual uint32          getSurfaceWidth(WindowSurfaceID surface_id) = 0;
		virtual uint32          getSurfaceHeight(WindowSurfaceID surface_id) = 0;
		virtual void            destroySurface(WindowSurfaceID surface_id) = 0;

		[[nodiscard]] virtual const Device &getDevice(uint32 device_index) const = 0;
		[[nodiscard]] virtual uint32        getDeviceCount() const = 0;
		[[nodiscard]] virtual bool          deviceSupportsPresent(uint32 device_index, WindowSurfaceID surface_id) const = 0;

	private:
		std::unordered_map<WindowID, WindowSurfaceID> m_windowSurfaceMap; // Maps a window to a corresponding API-specific surface;
	};
}
