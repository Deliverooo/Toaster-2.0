#pragma once

#include "events/event.hpp"

namespace tst
{
	class WindowClosedEvent final : public Event
	{
	public:
		WindowClosedEvent() = default;

		EVENT_CLASS_CATEGORY(eEventCategoryApplication)
		EVENT_CLASS_TYPE(eWindowClose)

		[[nodiscard]] String toStr() const override
		{
			return "[Window Closed]";
		}
	};

	class WindowResizedEvent final : public Event
	{
	public:
		WindowResizedEvent(const uint32 width, const uint32 height) : m_width(width), m_height(height)
		{
		}

		EVENT_CLASS_CATEGORY(eEventCategoryApplication)
		EVENT_CLASS_TYPE(eWindowResize)

		[[nodiscard]] uint32 getWidth() const { return m_width; }
		[[nodiscard]] uint32 getHeight() const { return m_height; }

		[[nodiscard]] String toStr() const override
		{
			std::ostringstream ss;
			ss << "Window Resized Event -> [" << m_width << "," << m_height << "]";
			return ss.str();
		}

	private:
		uint32_t m_width;
		uint32_t m_height;
	};

	class WindowMinimizedEvent final : public Event
	{
	public:
		WindowMinimizedEvent(const bool minimized) : m_minimized(minimized)
		{
		}

		EVENT_CLASS_CATEGORY(eEventCategoryApplication)
		EVENT_CLASS_TYPE(eWindowMinimize)

		[[nodiscard]] bool isMinimized() const { return m_minimized; }

		[[nodiscard]] std::string toStr() const override
		{
			std::ostringstream ss;
			ss << "Window Minimized Event -> [" << m_minimized << "]";
			return ss.str();
		}

	private:
		bool m_minimized = false;
	};

	class WindowTitleBarHitTestEvent final : public Event
	{
	public:
		WindowTitleBarHitTestEvent(const int x, const int y, int &hit) : m_x(x), m_y(y), m_hit(hit)
		{
		}

		EVENT_CLASS_TYPE(eWindowTitleBarHitTest)
		EVENT_CLASS_CATEGORY(eEventCategoryApplication)

		[[nodiscard]] int getX() const { return m_x; }
		[[nodiscard]] int getY() const { return m_y; }
		void              setHit(const bool hit) const { m_hit = static_cast<int>(hit); }

		[[nodiscard]] std::string toStr() const override
		{
			std::ostringstream ss;
			ss << "Title Bar Hit Test Event -> [" << m_x << "," << m_y << "]";
			return ss.str();
		}

	private:
		int  m_x;
		int  m_y;
		int &m_hit;
	};
}
