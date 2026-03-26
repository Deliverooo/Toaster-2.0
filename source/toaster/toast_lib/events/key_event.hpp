#pragma once

#include "event.hpp"
#include "../input_codes.hpp"

namespace toaster
{
	class KeyEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Keyboard)

		[[nodiscard]] input::EKeyCode getKeyCode() const { return m_keyCode; }

	protected:
		explicit KeyEvent(const input::EKeyCode keycode) : m_keyCode(keycode)
		{
		}

		input::EKeyCode m_keyCode;
	};

	class KeyPressEvent final : public KeyEvent
	{
	public:
		KeyPressEvent(const input::EKeyCode keycode, const int repeatCount) : KeyEvent(keycode), m_repeatCount(repeatCount)
		{
		}

		EVENT_CLASS_TYPE(KeyPressed)

		int getRepeatCount() const { return m_repeatCount; }

		[[nodiscard]] virtual std::string toStr() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << static_cast<int>(m_keyCode) << "/" << m_repeatCount;
			return ss.str();
		}

	private:
		int m_repeatCount;
	};

	class KeyTypeEvent final : public KeyEvent
	{
	public:
		explicit KeyTypeEvent(const input::EKeyCode keycode) : KeyEvent(keycode)
		{
		}

		EVENT_CLASS_TYPE(KeyTyped)

		[[nodiscard]] virtual std::string toStr() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << static_cast<int>(m_keyCode);
			return ss.str();
		}
	};

	class KeyReleaseEvent final : public KeyEvent
	{
	public:
		explicit KeyReleaseEvent(const input::EKeyCode keycode) : KeyEvent(keycode)
		{
		}

		EVENT_CLASS_TYPE(KeyReleased)

		[[nodiscard]] virtual std::string toStr() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << static_cast<int>(m_keyCode);
			return ss.str();
		}
	};
}
