/*!
 * @file event.hpp
 */
#pragma once

#include "../string.hpp"
#include "../system_types.h"
#include "../util_defines.hpp"

#include <functional>

namespace toaster
{
	enum class EventType
	{
		None = 0,
		WindowClose,
		WindowMinimize,
		WindowMaximize,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved,
		AppTick,
		AppUpdate,
		AppRender,
		KeyPressed,
		KeyReleased,
		KeyTyped,
		MouseButtonPressed,
		MouseButtonReleased,
		MouseButtonDown,
		MouseMoved,
		MouseScrolled
	};

	enum EventCategory
	{
		None                      = 0,
		EventCategory_Application = BIT(0),
		EventCategory_Input       = BIT(1),
		EventCategory_Keyboard    = BIT(2),
		EventCategory_Mouse       = BIT(3),
		EventCategory_MouseButton = BIT(4)
	};

	#define EVENT_CLASS_TYPE(__type) static EventType getStaticType() {return EventType::__type;}\
							   EventType getEventType() const override {return getStaticType();}\
							   CString getEventName() const override {return #__type;}

	#define EVENT_CLASS_CATEGORY(__category) virtual int32 getEventCategory() const override {return __category;}

	/*!
	 * @class Event
	 * @brief An abstract class defining the outline of what an event should have.
	 */
	class Event
	{
	public:
		virtual ~Event() = default;

		[[nodiscard]] virtual EventType getEventType() const = 0;

		[[nodiscard]] virtual CString getEventName() const = 0;

		[[nodiscard]] virtual int32 getEventCategory() const = 0;

		[[nodiscard]] virtual String toStr() const = 0;

		[[nodiscard]] bool inCategory(const EventCategory p_category) const { return getEventCategory() & p_category; }

		[[nodiscard]] bool isHandled() const { return m_handled; }
		void               setHandled(const bool p_handled) { m_handled = p_handled; }

	protected:
		bool m_handled = false;

		friend class EventDispatcher;
	};

	// std::bind is not good, so I use this instead...
	#define TST_BIND_EVENT_FN(__func) [this](auto &p_event) mutable -> bool { return __func(p_event); }

	template<typename Type>
	using EventFunc = std::function<bool(Type &)>;

	// EventDispatcher is a utility class that allows for easy dispatching of events
	// It takes an event and a function, and calls the function if the event type matches
	class EventDispatcher
	{
	public:
		// Constructor that takes an event reference
		explicit EventDispatcher(Event &p_event) : m_event(p_event)
		{
		}

		// Dispatch function that takes a function and calls it if the event type matches
		template<typename Type> requires std::derived_from<Type, Event>
		bool dispatch(EventFunc<Type> p_func)
		{
			// Check if the event type matches the type T
			if (m_event.getEventType() == Type::getStaticType())
			{
				// Cast the event to type T and call the function with it
				m_event.m_handled = p_func(static_cast<Type &>(m_event));
				return true;
			}

			return false;
		}

	private:
		Event &m_event;
	};

	using EventCallbackFn = std::function<void(Event &)>;
}
