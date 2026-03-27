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
		EVENT_CLASS_TYPE(WindowClose)

		[[nodiscard]] String toStr() const override
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
		EVENT_CLASS_TYPE(WindowResize)

		[[nodiscard]] uint32  getWidth() const { return m_width; }
		[[nodiscard]] uint32  getHeight() const { return m_height; }
		[[nodiscard]] float32 getAspectRatio() const { return static_cast<float32>(m_width) / static_cast<float32>(m_height); }

		[[nodiscard]] String toStr() const override
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
		EVENT_CLASS_TYPE(WindowMinimize)

		[[nodiscard]] bool isMinimized() const { return m_minimized; }

		[[nodiscard]] std::string toStr() const override
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
		EVENT_CLASS_TYPE(WindowMaximize)

		[[nodiscard]] bool isMaximized() const { return m_maximized; }

		[[nodiscard]] String toStr() const override
		{
			return "Window Minimized Event -> [" + to_string(m_maximized) + "]";
		}

	private:
		bool m_maximized;
	};
}
