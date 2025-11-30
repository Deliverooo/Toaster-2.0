#pragma once

#include "core_typedefs.hpp"
#include "core_defines.hpp"
#include "string/string.hpp"

#include <functional>

namespace tst
{
	enum class EEventType
	{
		eNone = 0,
		eWindowClose,
		eWindowMinimize,
		eWindowResize,
		eWindowFocus,
		eWindowLostFocus,
		eWindowMoved,
		eWindowTitleBarHitTest,
		eKeyPressed,
		eKeyReleased,
		eKeyTyped,
		eMouseButtonPressed,
		eMouseButtonReleased,
		eMouseButtonDown,
		eMouseMoved,
		eMouseScrolled
	};

	enum EEventCategory
	{
		eNone                     = 0,
		eEventCategoryApplication = BIT(0),
		eEventCategoryInput       = BIT(1),
		eEventCategoryKeyboard    = BIT(2),
		eEventCategoryMouse       = BIT(3),
		eEventCategoryMouseButton = BIT(4)
	};

	#define EVENT_CLASS_TYPE(type) static EEventType getStaticType() {return EEventType::type;}\
							   EEventType getEventType() const override {return getStaticType();}\
							   const char* getEventName() const override {return #type;}

	#define EVENT_CLASS_CATEGORY(category) virtual int getEventCategory() const override {return category;}

	// an abstract class defining the outline of what an event should have
	class Event
	{
	public:
		virtual ~Event() = default;

		[[nodiscard]] virtual EEventType  getEventType() const = 0;
		[[nodiscard]] virtual const char *getEventName() const = 0;
		[[nodiscard]] virtual int         getEventCategory() const = 0;
		[[nodiscard]] virtual String      toStr() const = 0;

		[[nodiscard]] bool inCategory(const EEventCategory category) const { return getEventCategory() & category; }

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
		EventDispatcher(Event &event) : m_event(event)
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

	using FnEventCallback = std::function<void(Event &)>;
}
