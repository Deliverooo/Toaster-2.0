#pragma once

#include <utility> // std::pair

#include "toast_lib/input_codes.hpp"
#include "toast_lib/system_types.h"

struct GLFWwindow;

namespace toaster::input
{
	void setCurrentWindowContext(GLFWwindow *p_window_ctx);

	void        setCursorMode(ECursorMode p_mode);
	ECursorMode getCursorMode();

	float32                     getMouseX();
	float32                     getMouseY();
	std::pair<float32, float32> getMousePos();

	bool isMouseButtonDown(EMouseButton p_button);
	bool isKeyDown(EKeyCode p_key_code);
}
