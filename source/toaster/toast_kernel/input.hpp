#pragma once

#include <utility> // std::pair

#include "toast_lib/input_codes.hpp"
#include "toast_lib/system_types.h"

struct GLFWwindow;

namespace toaster::input
{
	auto setCurrentWindowContext(GLFWwindow *p_window_ctx) -> void;

	auto setCursorMode(ECursorMode p_mode) -> void;
	auto getCursorMode() -> ECursorMode;

	auto getMouseX() -> float32;
	auto getMouseY() -> float32;
	auto getMousePos() -> std::pair<float32, float32>;

	auto isMouseButtonDown(EMouseButton p_button) -> bool;
	auto isKeyDown(EKeyCode p_key_code) -> bool;
}
