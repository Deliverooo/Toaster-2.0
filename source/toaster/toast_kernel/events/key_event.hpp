#pragma once

#include "event.hpp"
#include "keycodes.hpp"

namespace toaster
{
	class KeyEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Keyboard)

		[[nodiscard]] KeyCode getKeyCode() const { return m_keyCode; }

	protected:
		explicit KeyEvent(const KeyCode keycode) : m_keyCode(keycode)
		{
		}

		KeyCode m_keyCode;
	};

	class KeyPressedEvent final : public KeyEvent
	{
	public:
		KeyPressedEvent(const KeyCode keycode, const int repeatCount) : KeyEvent(keycode), m_repeatCount(repeatCount)
		{
		}

		EVENT_CLASS_TYPE(KeyPressed)

		int getRepeatCount() const { return m_repeatCount; }

		[[nodiscard]] virtual std::string toStr() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_keyCode << "/" << m_repeatCount;
			return ss.str();
		}

	private:
		int m_repeatCount;
	};

	class KeyTypedEvent final : public KeyEvent
	{
	public:
		explicit KeyTypedEvent(const KeyCode keycode) : KeyEvent(keycode)
		{
		}

		EVENT_CLASS_TYPE(KeyTyped)

		[[nodiscard]] virtual std::string toStr() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_keyCode;
			return ss.str();
		}
	};

	class KeyReleasedEvent final : public KeyEvent
	{
	public:
		explicit KeyReleasedEvent(const KeyCode keycode) : KeyEvent(keycode)
		{
		}

		EVENT_CLASS_TYPE(KeyReleased)

		[[nodiscard]] virtual std::string toStr() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_keyCode;
			return ss.str();
		}
	};
}
