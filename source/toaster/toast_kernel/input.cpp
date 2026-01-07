#include "input.hpp"

#include <GLFW/glfw3.h>

namespace toaster::input
{
	static GLFWwindow *s_currentWindow = nullptr;

	void setCurrentWindowContext(GLFWwindow *p_window_ctx)
	{
		s_currentWindow = p_window_ctx;
	}

	float32 getMouseX()
	{
		auto [x, y] = getMousePos();
		return x;
	}

	float32 getMouseY()
	{
		auto [x, y] = getMousePos();
		return y;
	}

	std::pair<float32, float32> getMousePos()
	{
		float64 x;
		float64 y;
		glfwGetCursorPos(s_currentWindow, &x, &y);
		return std::make_pair(static_cast<float32>(x), static_cast<float32>(y));
	}

	bool isMouseButtonDown(EMouseButton p_button)
	{
		const auto state = glfwGetMouseButton(s_currentWindow, static_cast<int32_t>(p_button));
		return state == GLFW_PRESS;
	}

	bool isKeyDown(EKeyCode p_key_code)
	{
		const auto state = glfwGetKey(s_currentWindow, static_cast<int32_t>(p_key_code));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	std::pair<float32, float32> getMousePosition()
	{
		float64 x, y;
		glfwGetCursorPos(s_currentWindow, &x, &y);
		return {static_cast<float>(x), static_cast<float>(y)};
	}

	void setCursorMode(ECursorMode p_mode)
	{
		glfwSetInputMode(s_currentWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL + static_cast<int>(p_mode));
	}

	ECursorMode getCursorMode()
	{
		return static_cast<ECursorMode>(glfwGetInputMode(s_currentWindow, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}
}
