#pragma once

#include "../toaster_export.hpp"

#include <utility> // std::pair

#include "toast_lib/input_codes.hpp"
#include "toast_lib/system_types.h"

#include "toast_lib/ptr.hpp"

namespace toaster
{
	class Window;

	class TST_API InputContext
	{
	public:
		InputContext(Window *p_window);

		auto setCursorMode(input::ECursorMode p_mode) -> void;
		auto getCursorMode() -> input::ECursorMode;

		auto getMouseX() -> float32;
		auto getMouseY() -> float32;
		auto getMousePos() -> std::pair<float32, float32>;

		auto isMouseButtonDown(input::EMouseButton p_button) -> bool;
		auto isKeyDown(input::EKeyCode p_key_code) -> bool;

	private:
		NonOwningPtr<Window> m_window{nullptr};
	};
}
