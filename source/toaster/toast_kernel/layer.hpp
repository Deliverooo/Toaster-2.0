/*!
 * @file layer.hpp
 */
#pragma once

#include "../toaster_macros.hpp"

#include "toast_lib/system_types.h"
#include "toast_lib/events/event.hpp"

namespace toaster
{
	class Application;

	/*!
	 * @class IAppLayer
	 * @brief Represents the interface for the application layer in a software architecture.
	 *
	 * The IAppLayer interface is designed to define a contract for the key functionalities
	 * required at the application layer. Classes implementing this interface should provide
	 * concrete implementations for the defined methods and ensure adherence to the specified
	 * behaviors.
	 *
	 * The application layer is responsible for handling logic, coordinating data
	 * flows, and communicating with underlying layers such as the data or domain layers.
	 * It acts as the central component for managing application-specific workflows and
	 * delivering responses to higher layers or external interfaces.
	 *
	 * @note This is an interface and, therefore, does not contain any implemented functionality.
	 *       All methods defined within this interface should be implemented in a derived class.
	 *
	 * @remarks Implementations of this interface should ensure thread safety where necessary
	 *          and follow design principles to promote scalability and maintainability.
	 */
	class TST_API IAppLayer
	{
	public:
		template<typename TLayer> requires std::derived_from<TLayer, IAppLayer>
		static auto alloc(Application *p_app) -> TLayer *
		{
			return new TLayer(p_app);
		}

		explicit IAppLayer(Application *p_app) : m_appParent(p_app)
		{
		}

		virtual ~IAppLayer();

		virtual auto onInit() -> void = 0;
		virtual auto onDestroy() -> void = 0;

		virtual auto onUpdate(float32 p_dt) -> void = 0;
		virtual auto onEvent(Event &p_event) -> void = 0;

		virtual auto onUIRender() -> void
		{
		}

		virtual auto getApp() -> Application & { return *m_appParent; }

	private:
		Application *m_appParent{nullptr};
	};
}
