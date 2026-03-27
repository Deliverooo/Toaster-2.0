#pragma once

#include "event.hpp"
#include "../input_codes.hpp"

namespace toaster
{
	class MouseMoveEvent final : public Event
	{
	public:
		MouseMoveEvent(const float32 p_x, const float32 p_y) : m_mouseX(p_x), m_mouseY(p_y)
		{
		}

		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse)
		EVENT_CLASS_TYPE(MouseMoved)

		[[nodiscard]] float32 getMouseX() const { return m_mouseX; }
		[[nodiscard]] float32 getMouseY() const { return m_mouseY; }

		[[nodiscard]] String toStr() const override
		{
			return "Mouse Move Event -> [" +to_string(m_mouseX) + "," + to_string(m_mouseY) + "]";
		}

	private:
		float32 m_mouseX;
		float32 m_mouseY;
	};

	class MouseScrollEvent final : public Event
	{
	public:
		MouseScrollEvent(const float32 p_x_offset, const float32 p_y_offset) : m_scrollX(p_x_offset), m_scrollY(p_y_offset)
		{
		}

		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse)
		EVENT_CLASS_TYPE(MouseScrolled)

		[[nodiscard]] float32 getScrollX() const { return m_scrollX; }
		[[nodiscard]] float32 getScrollY() const { return m_scrollY; }

		[[nodiscard]] String toStr() const override
		{
			return "Mouse Scroll Event -> [" + to_string(m_scrollX) + "," + to_string(m_scrollY) + "]";
		}

	private:
		float32 m_scrollX;
		float32 m_scrollY;
	};

	class MouseButtonEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse)

		[[nodiscard]] input::EMouseButton getMouseButton() const { return m_mouseButton; }

	protected:
		explicit MouseButtonEvent(const input::EMouseButton button) : m_mouseButton(button)
		{
		}

		input::EMouseButton m_mouseButton;
	};

	class MouseButtonPressEvent final : public MouseButtonEvent
	{
	public:
		explicit MouseButtonPressEvent(const input::EMouseButton p_button) : MouseButtonEvent(p_button)
		{
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)

		[[nodiscard]] String toStr() const override
		{
			return "Mouse Button Pressed Event -> [" + to_string(static_cast<uint16>(m_mouseButton)) + "]";
		}
	};

	class MouseButtonReleaseEvent final : public MouseButtonEvent
	{
	public:
		explicit MouseButtonReleaseEvent(const input::EMouseButton p_button) : MouseButtonEvent(p_button)
		{
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)

		[[nodiscard]] String toStr() const override
		{
			return "Mouse Button Released Event -> [" + to_string(static_cast<uint16>(m_mouseButton)) + "]";
		}
	};

	class MouseButtonDownEvent final : public MouseButtonEvent
	{
	public:
		explicit MouseButtonDownEvent(const input::EMouseButton p_button)
			: MouseButtonEvent(p_button)
		{
		}

		[[nodiscard]] String toStr() const override
		{
			return "Mouse Button Down Event -> [" + to_string(static_cast<uint16>(m_mouseButton)) + "]";
		}

		EVENT_CLASS_TYPE(MouseButtonDown)
	};
}
