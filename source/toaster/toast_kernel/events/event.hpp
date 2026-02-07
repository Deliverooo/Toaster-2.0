#pragma once

#include "util_defines.hpp"

#include <functional>
#include <sstream>
#include <string>

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

	#define EVENT_CLASS_TYPE(type) static EventType getStaticType() {return EventType::type;}\
							   EventType getEventType() const override {return getStaticType();}\
							   const char* getEventName() const override {return #type;}

	#define EVENT_CLASS_CATEGORY(category) virtual int getEventCategory() const override {return category;}

	// an abstract class defining the outline of what an event should have
	class Event
	{
	public:
		virtual ~Event() = default;

		[[nodiscard]] virtual EventType getEventType() const = 0;

		[[nodiscard]] virtual const char *getEventName() const = 0;

		[[nodiscard]] virtual int getEventCategory() const = 0;

		[[nodiscard]] virtual std::string toStr() const = 0;

		[[nodiscard]] bool inCategory(const EventCategory category) const { return getEventCategory() & category; }

		[[nodiscard]] bool isHandled() const { return m_handled; }
		void               setHandled(const bool handled) { m_handled = handled; }

	protected:
		bool m_handled = false;

		friend class EventDispatcher;
	};

	template<typename T>
	using EventFunc = std::function<bool(T &)>;

	// EventDispatcher is a utility class that allows for easy dispatching of events
	// It takes an event and a function, and calls the function if the event type matches
	class EventDispatcher
	{
	public:
		// Constructor that takes an event reference
		explicit EventDispatcher(Event &event) : m_event(event)
		{
		}

		// Dispatch function that takes a function and calls it if the event type matches
		template<typename T> requires std::derived_from<T, Event>
		bool dispatch(EventFunc<T> func)
		{
			// Check if the event type matches the type T
			if (m_event.getEventType() == T::getStaticType())
			{
				// Cast the event to type T and call the function with it
				m_event.m_handled = func(static_cast<T &>(m_event));
				return true;
			}

			return false;
		}

	private:
		Event &m_event;
	};

	using EventCallbackFn = std::function<void(Event &)>;
}
