#pragma once

#include "event.hpp"
#include "toast_lib/input_codes.hpp"

namespace toaster
{
	class TST_LIB_API MouseMoveEvent final : public Event
	{
	public:
		MouseMoveEvent(const float32 p_x, const float32 p_y) : m_mouseX(p_x), m_mouseY(p_y)
		{
		}

		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse)
		EVENT_CLASS_TYPE(eMouseMoved)

		[[nodiscard]] auto getMouseX() const -> float32 { return m_mouseX; }
		[[nodiscard]] auto getMouseY() const -> float32 { return m_mouseY; }

		[[nodiscard]] virtual auto toStr() const -> String override
		{
			return "Mouse Move Event -> [" + to_string(m_mouseX) + "," + to_string(m_mouseY) + "]";
		}

	private:
		float32 m_mouseX;
		float32 m_mouseY;
	};

	class TST_LIB_API MouseScrollEvent final : public Event
	{
	public:
		MouseScrollEvent(const float32 p_x_offset, const float32 p_y_offset) : m_scrollX(p_x_offset), m_scrollY(p_y_offset)
		{
		}

		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse)
		EVENT_CLASS_TYPE(eMouseScrolled)

		[[nodiscard]] auto getScrollX() const -> float32 { return m_scrollX; }
		[[nodiscard]] auto getScrollY() const -> float32 { return m_scrollY; }

		[[nodiscard]] virtual auto toStr() const -> String override
		{
			return "Mouse Scroll Event -> [" + to_string(m_scrollX) + "," + to_string(m_scrollY) + "]";
		}

	private:
		float32 m_scrollX;
		float32 m_scrollY;
	};

	class TST_LIB_API MouseButtonEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse)

		[[nodiscard]] auto getMouseButton() const -> input::EMouseButton { return m_mouseButton; }

	protected:
		explicit MouseButtonEvent(const input::EMouseButton button) : m_mouseButton(button)
		{
		}

		input::EMouseButton m_mouseButton;
	};

	class TST_LIB_API MouseButtonPressEvent final : public MouseButtonEvent
	{
	public:
		explicit MouseButtonPressEvent(const input::EMouseButton p_button) : MouseButtonEvent(p_button)
		{
		}

		EVENT_CLASS_TYPE(eMouseButtonPressed)

		[[nodiscard]] virtual auto toStr() const -> String override
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

		EVENT_CLASS_TYPE(eMouseButtonReleased)

		[[nodiscard]] virtual auto toStr() const -> String override
		{
			return "Mouse Button Released Event -> [" + to_string(static_cast<uint16>(m_mouseButton)) + "]";
		}
	};
}
