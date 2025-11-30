#pragma once
#include "events/event.hpp"
#include "core/misc/key_codes.hpp"

namespace tst
{
	class MouseMoveEvent final : public Event
	{
	public:
		MouseMoveEvent(const float x, const float y) : m_mouseX(x), m_mouseY(y)
		{
		}

		EVENT_CLASS_CATEGORY(eEventCategoryInput | eEventCategoryMouse)
		EVENT_CLASS_TYPE(eMouseMoved)

		float getMouseX() const { return m_mouseX; }
		float getMouseY() const { return m_mouseY; }

		virtual String toStr() const override
		{
			std::ostringstream oss;
			oss << "Mouse Move Event -> [" << m_mouseX << "," << m_mouseY << "]";
			return oss.str();
		}

	private:
		float m_mouseX;
		float m_mouseY;
	};

	class MouseScrollEvent final : public Event
	{
	public:
		MouseScrollEvent(float xOffset, float yOffset) : m_scrollX(xOffset), m_scrollY(yOffset)
		{
		}

		EVENT_CLASS_CATEGORY(eEventCategoryInput | eEventCategoryMouse)
		EVENT_CLASS_TYPE(eMouseScrolled)

		[[nodiscard]] float getScrollX() const { return m_scrollX; }
		[[nodiscard]] float getScrollY() const { return m_scrollY; }

		String toStr() const override
		{
			std::ostringstream oss;
			oss << "Mouse Scroll Event -> [" << m_scrollX << "," << m_scrollY << "]";
			return oss.str();
		}

	private:
		float m_scrollX;
		float m_scrollY;
	};

	class MouseButtonEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(eEventCategoryInput | eEventCategoryMouse)

		EMouseButton getMouseButton() const { return m_mouseButton; }

	protected:
		MouseButtonEvent(EMouseButton button) : m_mouseButton(button)
		{
		}

		EMouseButton m_mouseButton;
	};

	class MouseButtonPressEvent final : public MouseButtonEvent
	{
	public:
		MouseButtonPressEvent(EMouseButton button) : MouseButtonEvent(button)
		{
		}

		EVENT_CLASS_TYPE(eMouseButtonPressed)

		String toStr() const override
		{
			std::ostringstream oss;
			oss << "Mouse Button Pressed Event -> [" << static_cast<uint8>(m_mouseButton) << "]";
			return oss.str();
		}
	};

	class MouseButtonReleaseEvent final : public MouseButtonEvent
	{
	public:
		MouseButtonReleaseEvent(EMouseButton button) : MouseButtonEvent(button)
		{
		}

		EVENT_CLASS_TYPE(eMouseButtonReleased)

		String toStr() const override
		{
			std::ostringstream oss;
			oss << "Mouse Button Released Event -> [" << static_cast<uint8>(m_mouseButton) << "]";
			return oss.str();
		}
	};

	class MouseButtonDownEvent final : public MouseButtonEvent
	{
	public:
		MouseButtonDownEvent(EMouseButton button)
			: MouseButtonEvent(button)
		{
		}

		String toStr() const override
		{
			std::ostringstream oss;
			oss << "Mouse Button Down Event -> [" << static_cast<uint8>(m_mouseButton) << "]";
			return oss.str();
		}

		EVENT_CLASS_TYPE(eMouseButtonDown)
	};
}
