/*!
 * @file window_event.hpp
 */
#pragma once

#include "event.hpp"

namespace toaster
{
	class WindowCloseEvent final : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_CATEGORY(EventCategory_Application)
		EVENT_CLASS_TYPE(eWindowClose)

		[[nodiscard]] virtual auto toStr() const -> String override
		{
			return "Window Closed Event -> [Window Closed]";
		}
	};

	class WindowResizeEvent final : public Event
	{
	public:
		WindowResizeEvent(const uint32 width, const uint32 height) : m_width(width), m_height(height)
		{
		}

		EVENT_CLASS_CATEGORY(EventCategory_Application)
		EVENT_CLASS_TYPE(eWindowResize)

		[[nodiscard]] auto getWidth() const -> uint32 { return m_width; }
		[[nodiscard]] auto getHeight() const -> uint32 { return m_height; }
		[[nodiscard]] auto getAspectRatio() const -> float32 { return static_cast<float32>(m_width) / static_cast<float32>(m_height); }

		[[nodiscard]] virtual auto toStr() const -> String override
		{
			return "Window Resize Event -> [" + to_string(m_width) + ", " + to_string(m_height) + "]";
		}

	private:
		uint32 m_width;
		uint32 m_height;
	};

	class WindowMinimizeEvent final : public Event
	{
	public:
		explicit WindowMinimizeEvent(const bool minimized) : m_minimized(minimized)
		{
		}

		EVENT_CLASS_CATEGORY(EventCategory_Application)
		EVENT_CLASS_TYPE(eWindowMinimize)

		[[nodiscard]] auto isMinimized() const -> bool { return m_minimized; }

		[[nodiscard]] virtual auto toStr() const -> String override
		{
			return "Window Minimized Event -> [" + to_string(m_minimized) + "]";
		}

	private:
		bool m_minimized;
	};

	class WindowMaximizeEvent final : public Event
	{
	public:
		explicit WindowMaximizeEvent(const bool p_maximized) : m_maximized(p_maximized)
		{
		}

		EVENT_CLASS_CATEGORY(EventCategory_Application)
		EVENT_CLASS_TYPE(eWindowMaximize)

		[[nodiscard]] auto isMaximized() const -> bool { return m_maximized; }

		[[nodiscard]] virtual auto toStr() const -> String override
		{
			return "Window Minimized Event -> [" + to_string(m_maximized) + "]";
		}

	private:
		bool m_maximized{true};
	};
}
