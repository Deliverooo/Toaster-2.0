/*!
 * @file layer.hpp
 */
#pragma once

#include "toast_kernel.hpp"
#include "toast_lib/ptr.hpp"

#include "toast_lib/system_types.h"
#include "toast_lib/events/event.hpp"

namespace toaster
{
	class Application;
	class InputContext;

	namespace render
	{
		class RenderContext;
		class Globals;
	}

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
	class TST_KERNEL_API IAppLayer
	{
	public:
		virtual ~IAppLayer() = default;

		virtual auto onInit() -> void = 0;

		virtual auto onDestroy() -> void
		{
		}

		virtual auto onUpdate([[maybe_unused]] float32 p_dt) -> void
		{
		}

		virtual auto onEvent([[maybe_unused]] Event &p_event) -> void
		{
		}

		// The width and height here are directly from the swapchain, not the OnWindowResizeEvent!.. Ts means that it is actually faster... Probably...
		virtual auto onResize([[maybe_unused]] tsm::uint2 p_size) -> void
		{
		}

		virtual auto onUIInit([[maybe_unused]] void *p_user_data) -> void
		{
		}

		virtual auto onUIRender() -> void
		{
		}

	protected:
		// Ts just makes things easier to acess
		NonOwningPtr<Application>           m_app{nullptr};
		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};
		NonOwningPtr<const render::Globals> m_globals{nullptr};
		NonOwningPtr<InputContext>          m_inputCtx{nullptr};

	private:
		auto _register(Application *p_app) -> void;
		friend class Application;
	};
}
