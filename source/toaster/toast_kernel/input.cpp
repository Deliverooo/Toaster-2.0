#include "input.hpp"

#include "window.hpp"
#include <GLFW/glfw3.h>

namespace toaster::input
{
	static Window *s_currentWindow{nullptr};

	auto setCurrentWindowContext(Window *p_window_ctx) -> void
	{
		s_currentWindow = p_window_ctx;
	}

	auto getMouseX() -> float32
	{
		auto [x, y] = getMousePos();
		return x;
	}

	auto getMouseY() -> float32
	{
		auto [x, y] = getMousePos();
		return y;
	}

	auto getMousePos() -> std::pair<float32, float32>
	{
		float64 x{0.0f};
		float64 y{0.0f};
		glfwGetCursorPos(s_currentWindow->getNativeWindow(), &x, &y);
		return std::make_pair(static_cast<float32>(x), static_cast<float32>(y));
	}

	auto isMouseButtonDown(EMouseButton p_button) -> bool
	{
		const auto state = glfwGetMouseButton(s_currentWindow->getNativeWindow(), static_cast<int32>(p_button));
		return state == GLFW_PRESS;
	}

	auto isKeyDown(EKeyCode p_key_code) -> bool
	{
		const auto state = glfwGetKey(s_currentWindow->getNativeWindow(), static_cast<int32>(p_key_code));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	auto getMousePosition() -> std::pair<float32, float32>
	{
		float64 x{0.0f};
		float64 y{0.0f};
		glfwGetCursorPos(s_currentWindow->getNativeWindow(), &x, &y);
		return {static_cast<float32>(x), static_cast<float32>(y)};
	}

	auto setCursorMode(ECursorMode p_mode) -> void
	{
		glfwSetInputMode(s_currentWindow->getNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL + static_cast<int32>(p_mode));
	}

	auto getCursorMode() -> ECursorMode
	{
		return static_cast<ECursorMode>(glfwGetInputMode(s_currentWindow->getNativeWindow(), GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}
}
