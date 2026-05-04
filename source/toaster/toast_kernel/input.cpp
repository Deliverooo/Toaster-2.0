#include "input.hpp"

#include <GLFW/glfw3.h>
#include "window.hpp"
#include "toast_scripting/script_engine.hpp"

namespace toaster
{
	static InputContext *s_activeInputCtx{nullptr}; // For the script method lambdas

	InputContext::InputContext(Window *p_window) : m_window(p_window)
	{
		s_activeInputCtx = this;
	}

	InputContext::~InputContext()
	{
		s_activeInputCtx = nullptr;
	}

	auto InputContext::registerScriptMethods(script::ScriptEngine *p_engine) -> void
	{
		p_engine->registerMethod("Toaster.Input::IsKeyDown", +[](input::EKeyCode p_key_code) -> bool
		{
			return s_activeInputCtx->isKeyDown(p_key_code);
		});

		p_engine->registerMethod("Toaster.Input::IsMouseButtonDown", +[](input::EMouseButton p_mouse_button) -> bool
		{
			return s_activeInputCtx->isMouseButtonDown(p_mouse_button);
		});

		p_engine->registerMethod("Toaster.Input::GetCursorMode", +[](input::ECursorMode *p_cursor_mode) -> void
		{
			*p_cursor_mode = s_activeInputCtx->getCursorMode();
		});

		p_engine->registerMethod("Toaster.Input::SetCursorMode", +[](input::ECursorMode p_cursor_mode) -> void
		{
			s_activeInputCtx->setCursorMode(p_cursor_mode);
		});

		p_engine->registerMethod("Toaster.Input::GetMousePos", +[](MousePos *p_out_pos) -> void
		{
			*p_out_pos = s_activeInputCtx->getMousePos();
		});
	}

	auto InputContext::getMouseX() -> float32
	{
		auto [x, y] = getMousePos();
		return x;
	}

	auto InputContext::getMouseY() -> float32
	{
		auto [x, y] = getMousePos();
		return y;
	}

	auto InputContext::getMousePos() -> MousePos
	{
		float64 x{0.0f};
		float64 y{0.0f};
		glfwGetCursorPos(m_window->getNativeWindow(), &x, &y);
		return std::make_pair(static_cast<float32>(x), static_cast<float32>(y));
	}

	auto InputContext::isMouseButtonDown(input::EMouseButton p_button) -> bool
	{
		const auto state = glfwGetMouseButton(m_window->getNativeWindow(), static_cast<int32>(p_button));
		return state == GLFW_PRESS;
	}

	auto InputContext::isKeyDown(input::EKeyCode p_key_code) -> bool
	{
		const auto state = glfwGetKey(m_window->getNativeWindow(), static_cast<int32>(p_key_code));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	auto InputContext::setCursorMode(input::ECursorMode p_mode) -> void
	{
		glfwSetInputMode(m_window->getNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL + static_cast<int32>(p_mode));
	}

	auto InputContext::getCursorMode() -> input::ECursorMode
	{
		return static_cast<input::ECursorMode>(glfwGetInputMode(m_window->getNativeWindow(), GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}
}
