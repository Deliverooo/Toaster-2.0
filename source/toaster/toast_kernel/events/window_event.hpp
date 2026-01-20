#pragma once

#include "event.hpp"

namespace toaster
{
	class WindowClosedEvent final : public Event
	{
	public:
		WindowClosedEvent() = default;

		EVENT_CLASS_CATEGORY(EventCategory_Application)
		EVENT_CLASS_TYPE(WindowClose)

		[[nodiscard]] virtual std::string toStr() const override
		{
			return "Window Closed Event -> [Window Closed]";
		}
	};

	class WindowResizedEvent final : public Event
	{
	public:
		WindowResizedEvent(const uint32_t width, const uint32_t height) : m_width(width), m_height(height)
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

	class WindowMinimizedEvent final : public Event
	{
	public:
		explicit WindowMinimizedEvent(const bool minimized) : m_minimized(minimized)
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
}
