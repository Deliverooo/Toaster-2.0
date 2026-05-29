#pragma once

#include "toast_kernel.hpp"

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

	class TST_KERNEL_API InputContext
	{
	public:
		using MousePos    = std::pair<float32, float32>;
		using MouseScroll = std::pair<float32, float32>;

		InputContext(Window *p_window);
		~InputContext();

		auto registerScriptMethods(script::ScriptEngine *p_engine) -> void;

		auto               setCursorMode(input::ECursorMode p_mode) -> void;
		[[nodiscard]] auto getCursorMode() const -> input::ECursorMode;

		[[nodiscard]] auto getMouseX() const -> float32;
		[[nodiscard]] auto getMouseY() const -> float32;
		[[nodiscard]] auto getMousePos() const -> MousePos;
		[[nodiscard]] auto getMouseScrollX() const -> float32;
		[[nodiscard]] auto getMouseScrollY() const -> float32;
		[[nodiscard]] auto getMouseScroll() const -> MouseScroll;

		[[nodiscard]] auto isKeyDown(input::EKeyCode p_key_code) const -> bool;
		[[nodiscard]] auto isKeyPressed(input::EKeyCode p_key_code) const -> bool;
		[[nodiscard]] auto isKeyReleased(input::EKeyCode p_key_code) const -> bool;
		[[nodiscard]] auto isKeyHeld(input::EKeyCode p_key_code) const -> bool;

		[[nodiscard]] auto isMouseButtonDown(input::EMouseButton p_button) const -> bool;
		[[nodiscard]] auto isMouseButtonPressed(input::EMouseButton p_button) const -> bool;
		[[nodiscard]] auto isMouseButtonReleased(input::EMouseButton p_button) const -> bool;
		[[nodiscard]] auto isMouseButtonHeld(input::EMouseButton p_button) const -> bool;

	private:
		auto _setKeyState(input::EKeyCode p_key_code, input::EKeyState p_key_state) -> void;
		auto _setMouseButtonState(input::EMouseButton p_mouse_button, input::EKeyState p_key_state) -> void;
		auto _setMouseScroll(float32 p_x_offset, float32 p_y_offset) -> void;
		auto _update() -> void;
		auto _onEndFrame() -> void;

		NonOwningPtr<Window> m_window{nullptr};

		std::unordered_map<input::EKeyCode, input::EKeyState>     m_keyStateMap;
		std::unordered_map<input::EMouseButton, input::EKeyState> m_mouseButtonStateMap;
		MouseScroll                                               m_mouseScroll;

		friend class Window;
	};
}
