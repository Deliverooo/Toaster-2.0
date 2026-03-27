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
		explicit KeyEvent(const input::EKeyCode p_keycode) : m_keyCode(p_keycode)
		{
		}

		input::EKeyCode m_keyCode;
	};

	class KeyPressEvent final : public KeyEvent
	{
	public:
		KeyPressEvent(const input::EKeyCode p_keycode, const int32 p_repeat_count) : KeyEvent(p_keycode), m_repeatCount(p_repeat_count)
		{
		}

		EVENT_CLASS_TYPE(KeyPressed)

		[[nodiscard]] int32 getRepeatCount() const { return m_repeatCount; }

		[[nodiscard]] String toStr() const override
		{
			return "KeyPressedEvent: " +to_string(static_cast<int32>(m_keyCode)) + "/" + to_string(m_repeatCount);
		}

	private:
		int m_repeatCount;
	};

	class KeyTypeEvent final : public KeyEvent
	{
	public:
		explicit KeyTypeEvent(const input::EKeyCode p_keycode) : KeyEvent(p_keycode)
		{
		}

		EVENT_CLASS_TYPE(KeyTyped)

		[[nodiscard]] String toStr() const override
		{
			return "KeyTypedEvent: " + to_string(static_cast<int32>(m_keyCode));
		}
	};

	class KeyReleaseEvent final : public KeyEvent
	{
	public:
		explicit KeyReleaseEvent(const input::EKeyCode p_keycode) : KeyEvent(p_keycode)
		{
		}

		EVENT_CLASS_TYPE(KeyReleased)

		[[nodiscard]] String toStr() const override
		{
			return "KeyReleasedEvent: " + to_string(static_cast<int32>(m_keyCode));
		}
	};
}
