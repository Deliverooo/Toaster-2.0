#include "input.hpp"

#include <GLFW/glfw3.h>
#include "window.hpp"

namespace toaster
{
	InputContext::InputContext(Window *p_window) : m_window(p_window)
	{
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

	auto InputContext::getMousePos() -> std::pair<float32, float32>
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
