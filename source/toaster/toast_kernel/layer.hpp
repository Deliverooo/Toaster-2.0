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
		explicit IAppLayer(Application *p_app_parent) : m_appParent(p_app_parent)
		{
		}

		virtual ~IAppLayer() = default;

		virtual void onInit() = 0;
		virtual void onDestroy() = 0;

		virtual void onUpdate(float32 p_dt) = 0;
		virtual void onEvent(Event &p_event) = 0;

	protected:
		Application *m_appParent{nullptr};
	};
}
