#include "sandbox_layer.hpp"

#include "platform/application.hpp"

#include "events/key_event.hpp"
#include "events/mouse_event.hpp"
#include "events/window_event.hpp"
#include "platform/input.hpp"

namespace tst
{
	SandboxLayer::SandboxLayer()
	{
	}

	SandboxLayer::~SandboxLayer()
	{
	}

	void SandboxLayer::onInit()
	{
		Window &app_window = Application::getInstance().getMainWindow();
		app_window.setWindowFlag(EWindowFlags::eSharpCorners, true);
		app_window.setWindowMinSize({200, 200});
	}

	void SandboxLayer::onDestroy()
	{
	}

	void SandboxLayer::onUpdate(float32 dt)
	{
	}

	void SandboxLayer::onEvent(Event &event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<KeyPressedEvent>([](KeyPressedEvent &event)
		{
			Window &app_window = Application::getInstance().getMainWindow();
			// app_window.setTitle(getKeycodeString(event.getKeyCode()));

			switch (event.getKeyCode())
			{
				case EKeyCode::eE:
				{
				}
				case EKeyCode::eQ:
				{
				}
				default:
				{
				}
			}
			return false;
		});
	}

	void SandboxLayer::onGUIRender()
	{
	}
}
