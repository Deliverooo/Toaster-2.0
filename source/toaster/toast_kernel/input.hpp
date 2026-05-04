#pragma once

#include "../toaster_macros.hpp"

#include <utility> // std::pair

#include "toast_lib/input_codes.hpp"
#include "toast_lib/system_types.h"

#include "toast_lib/ptr.hpp"

namespace toaster
{
	class Window;

	namespace script
	{
		class ScriptEngine;
	}

	class TST_API InputContext
	{
	public:
		using MousePos = std::pair<float32, float32>;

		InputContext(Window *p_window);
		~InputContext();

		auto registerScriptMethods(script::ScriptEngine *p_engine) -> void;

		auto setCursorMode(input::ECursorMode p_mode) -> void;
		auto getCursorMode() -> input::ECursorMode;

		auto getMouseX() -> float32;
		auto getMouseY() -> float32;
		auto getMousePos() -> MousePos;

		auto isMouseButtonDown(input::EMouseButton p_button) -> bool;
		auto isKeyDown(input::EKeyCode p_key_code) -> bool;

	private:
		NonOwningPtr<Window> m_window{nullptr};
	};
}
