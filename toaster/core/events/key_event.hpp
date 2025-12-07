#pragma once
#include "events/event.hpp"
#include "key_codes.hpp"

namespace tst
{
	class KeyEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(eEventCategoryInput | eEventCategoryKeyboard)

		[[nodiscard]] EKeyCode getKeyCode() const { return m_keyCode; }

	protected:
		KeyEvent(EKeyCode _keycode) : m_keyCode(_keycode)
		{
		}

		EKeyCode m_keyCode;
	};

	class KeyPressedEvent final : public KeyEvent
	{
	public:
		KeyPressedEvent(EKeyCode _keycode, int _repeatCount) : KeyEvent(_keycode), m_repeatCount(_repeatCount)
		{
		}

		EVENT_CLASS_TYPE(eKeyPressed)

		int getRepeatCount() const { return m_repeatCount; }

		[[nodiscard]] virtual String toStr() const override
		{
			return {static_cast<char>(m_keyCode)};
		}

	private:
		int m_repeatCount;
	};

	class KeyTypedEvent final : public KeyEvent
	{
	public:
		KeyTypedEvent(EKeyCode _keycode) : KeyEvent(_keycode)
		{
		}

		EVENT_CLASS_TYPE(eKeyTyped)

		[[nodiscard]] virtual String toStr() const override
		{
			return {static_cast<char>(m_keyCode)};
		}
	};

	class KeyReleasedEvent final : public KeyEvent
	{
	public:
		KeyReleasedEvent(EKeyCode _keycode) : KeyEvent(_keycode)
		{
		}

		EVENT_CLASS_TYPE(eKeyReleased)

		[[nodiscard]] virtual String toStr() const override
		{
			return {static_cast<char>(m_keyCode)};
		}
	};
}
