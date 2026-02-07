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

		[[nodiscard]] virtual std::string toStr() const override
		{
			return "Window Closed Event -> [Window Closed]";
		}
	};

	class WindowResizeEvent final : public Event
	{
	public:
		WindowResizeEvent(const uint32_t width, const uint32_t height) : m_width(width), m_height(height)
		{
		}

		EVENT_CLASS_CATEGORY(EventCategory_Application)
		EVENT_CLASS_TYPE(WindowResize)

		[[nodiscard]] uint32_t getWidth() const { return m_width; }
		[[nodiscard]] uint32_t getHeight() const { return m_height; }

		[[nodiscard]] virtual std::string toStr() const override
		{
			std::ostringstream ss;
			ss << m_width << " " << m_height;
			return ss.str();
		}

	private:
		uint32_t m_width;
		uint32_t m_height;
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

		[[nodiscard]] virtual std::string toStr() const override
		{
			std::ostringstream ss;
			ss << "Window Minimized Event -> [" << m_minimized << "]";
			return ss.str();
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

		[[nodiscard]] virtual std::string toStr() const override
		{
			std::ostringstream ss;
			ss << "Window Minimized Event -> [" << m_maximized << "]";
			return ss.str();
		}

	private:
		bool m_maximized;
	};
}
