/*!
 * @file layer.hpp
 */
#pragma once

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
	class IAppLayer
	{
	public:
		template<typename TLayer> requires std::derived_from<TLayer, IAppLayer>
		static TLayer *alloc(Application *p_app)
		{
			return new TLayer(p_app);
		}

		explicit IAppLayer(Application *p_app) : m_appParent(p_app)
		{
		}

		virtual ~IAppLayer() = default;

		virtual void onInit() = 0;
		virtual void onDestroy() = 0;

		virtual void onUpdate(float32 p_dt) = 0;
		virtual void onEvent(Event &p_event) = 0;

		virtual void onUIRender()
		{
		}

		virtual Application &getApp() { return *m_appParent; }

	private:
		Application *m_appParent{nullptr};
	};
}
