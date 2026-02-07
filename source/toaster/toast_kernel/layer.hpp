#pragma once

#include "logging.hpp"
#include "system_types.h"
#include "events/event.hpp"

namespace toaster
{
	class Application;

	class IAppLayer
	{
	public:
		explicit IAppLayer(Application *p_app) : m_appParent(p_app)
		{
		}

		virtual ~IAppLayer() = default;

		virtual void onInit() = 0;
		virtual void onDestroy() = 0;

		virtual void onUpdate(float32 p_dt) = 0;
		virtual void onEvent(Event &p_event) = 0;

		virtual Application &getApp() { return *m_appParent; }

	private:
		Application *m_appParent{nullptr};
	};
}
