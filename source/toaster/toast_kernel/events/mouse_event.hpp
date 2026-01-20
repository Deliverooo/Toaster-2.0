#pragma once

#include "event.hpp"
#include "keycodes.hpp"

#include <string>

namespace toaster
{
	class MouseMoveEvent final : public Event
	{
	public:
		MouseMoveEvent(const float x, const float y) : m_mouseX(x), m_mouseY(y)
		{
		}

		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse)
		EVENT_CLASS_TYPE(MouseMoved)

		float getMouseX() const { return m_mouseX; }
		float getMouseY() const { return m_mouseY; }

		virtual std::string toStr() const override
		{
			std::ostringstream ss;
			ss << "Mouse Move Event -> [" << m_mouseX << "," << m_mouseY << "]";
			return ss.str();
		}

	private:
		float m_mouseX;
		float m_mouseY;
	};

	class MouseScrollEvent final : public Event
	{
	public:
		MouseScrollEvent(const float xOffset, const float yOffset) : m_scrollX(xOffset), m_scrollY(yOffset)
		{
		}

		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse)
		EVENT_CLASS_TYPE(MouseScrolled)

		float getScrollX() const { return m_scrollX; }
		float getScrollY() const { return m_scrollY; }

		virtual std::string toStr() const override
		{
			std::ostringstream ss;
			ss << "Mouse Scroll Event -> [" << m_scrollX << "," << m_scrollY << "]";
			return ss.str();
		}

	private:
		float m_scrollX;
		float m_scrollY;
	};

	class MouseButtonEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse)

		MouseButton getMouseButton() const { return m_mouseButton; }

	protected:
		explicit MouseButtonEvent(const MouseButton button) : m_mouseButton(button)
		{
		}

		MouseButton m_mouseButton;
	};

	class MouseButtonPressEvent final : public MouseButtonEvent
	{
	public:
		explicit MouseButtonPressEvent(const MouseButton button) : MouseButtonEvent(button)
		{
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)

		virtual std::string toStr() const override
		{
			std::ostringstream ss;
			ss << "Mouse Button Pressed Event -> [" << static_cast<uint16_t>(m_mouseButton) << "]";
			return ss.str();
		}
	};

	class MouseButtonReleaseEvent final : public MouseButtonEvent
	{
	public:
		explicit MouseButtonReleaseEvent(const MouseButton button) : MouseButtonEvent(button)
		{
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)

		virtual std::string toStr() const override
		{
			return std::string("Mouse Button Released Event -> [" + std::to_string(static_cast<uint16_t>(m_mouseButton)) + "]");
		}
	};

	class MouseButtonDownEvent final : public MouseButtonEvent
	{
	public:
		explicit MouseButtonDownEvent(const MouseButton button)
			: MouseButtonEvent(button)
		{
		}

		virtual std::string toStr() const override
		{
			return std::string("Mouse Button Down Event -> [" + std::to_string(static_cast<uint16_t>(m_mouseButton)) + "]");
		}

		EVENT_CLASS_TYPE(MouseButtonDown)
	};
}
