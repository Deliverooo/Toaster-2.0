#include "toast_kernel/input.hpp"
#include "toast_kernel/window.hpp"

#include "toast_script/script_engine.hpp"

#include <GLFW/glfw3.h>

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
		if (!p_engine)
			return;
		p_engine->registerMethod("Toaster.Input::IsKeyDown", +[](input::EKeyCode p_key_code) -> bool
		{
			return s_activeInputCtx->isKeyDown(p_key_code);
		});

		p_engine->registerMethod("Toaster.Input::IsKeyPressed", +[](input::EKeyCode p_key_code) -> bool
		{
			return s_activeInputCtx->isKeyPressed(p_key_code);
		});

		p_engine->registerMethod("Toaster.Input::IsKeyReleased", +[](input::EKeyCode p_key_code) -> bool
		{
			return s_activeInputCtx->isKeyReleased(p_key_code);
		});

		p_engine->registerMethod("Toaster.Input::IsKeyHeld", +[](input::EKeyCode p_key_code) -> bool
		{
			return s_activeInputCtx->isKeyHeld(p_key_code);
		});

		p_engine->registerMethod("Toaster.Input::IsMouseButtonDown", +[](input::EMouseButton p_mouse_button) -> bool
		{
			return s_activeInputCtx->isMouseButtonDown(p_mouse_button);
		});

		p_engine->registerMethod("Toaster.Input::IsMouseButtonPressed", +[](input::EMouseButton p_mouse_button) -> bool
		{
			return s_activeInputCtx->isMouseButtonPressed(p_mouse_button);
		});

		p_engine->registerMethod("Toaster.Input::IsMouseButtonReleased", +[](input::EMouseButton p_mouse_button) -> bool
		{
			return s_activeInputCtx->isMouseButtonReleased(p_mouse_button);
		});

		p_engine->registerMethod("Toaster.Input::IsMouseButtonHeld", +[](input::EMouseButton p_mouse_button) -> bool
		{
			return s_activeInputCtx->isMouseButtonHeld(p_mouse_button);
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

		p_engine->registerMethod("Toaster.Input::GetMouseScroll", +[](MouseScroll *p_out_scroll) -> void
		{
			*p_out_scroll = s_activeInputCtx->getMouseScroll();
		});
	}

	auto InputContext::setCursorMode(input::ECursorMode p_mode) -> void
	{
		glfwSetInputMode(m_window->getNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL + static_cast<int32>(p_mode));
	}

	auto InputContext::getCursorMode() const -> input::ECursorMode
	{
		return static_cast<input::ECursorMode>(glfwGetInputMode(m_window->getNativeWindow(), GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}

	auto InputContext::getMouseX() const -> float32
	{
		auto [x, y] = getMousePos();
		return x;
	}

	auto InputContext::getMouseY() const -> float32
	{
		auto [x, y] = getMousePos();
		return y;
	}

	auto InputContext::getMousePos() const -> MousePos
	{
		float64 x{0.0f};
		float64 y{0.0f};
		glfwGetCursorPos(m_window->getNativeWindow(), &x, &y);
		return std::make_pair(static_cast<float32>(x), static_cast<float32>(y));
	}

	auto InputContext::getMouseScrollX() const -> float32
	{
		return m_mouseScroll.first;
	}

	auto InputContext::getMouseScrollY() const -> float32
	{
		return m_mouseScroll.second;
	}

	auto InputContext::getMouseScroll() const -> MouseScroll
	{
		return m_mouseScroll;
	}

	auto InputContext::isKeyDown(input::EKeyCode p_key_code) const -> bool
	{
		const auto state = glfwGetKey(m_window->getNativeWindow(), static_cast<int32>(p_key_code));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	auto InputContext::isKeyPressed(input::EKeyCode p_key_code) const -> bool
	{
		return m_keyStateMap.contains(p_key_code) && m_keyStateMap.at(p_key_code) == input::EKeyState::ePressed;
	}

	auto InputContext::isKeyReleased(input::EKeyCode p_key_code) const -> bool
	{
		return m_keyStateMap.contains(p_key_code) && m_keyStateMap.at(p_key_code) == input::EKeyState::eReleased;
	}

	auto InputContext::isKeyHeld(input::EKeyCode p_key_code) const -> bool
	{
		return m_keyStateMap.contains(p_key_code) && m_keyStateMap.at(p_key_code) == input::EKeyState::eHeld;
	}

	auto InputContext::isMouseButtonDown(input::EMouseButton p_button) const -> bool
	{
		const auto state = glfwGetMouseButton(m_window->getNativeWindow(), static_cast<int32>(p_button));
		return state == GLFW_PRESS;
	}

	auto InputContext::isMouseButtonPressed(input::EMouseButton p_button) const -> bool
	{
		return m_mouseButtonStateMap.contains(p_button) && m_mouseButtonStateMap.at(p_button) == input::EKeyState::ePressed;
	}

	auto InputContext::isMouseButtonReleased(input::EMouseButton p_button) const -> bool
	{
		return m_mouseButtonStateMap.contains(p_button) && m_mouseButtonStateMap.at(p_button) == input::EKeyState::eReleased;
	}

	auto InputContext::isMouseButtonHeld(input::EMouseButton p_button) const -> bool
	{
		return m_mouseButtonStateMap.contains(p_button) && m_mouseButtonStateMap.at(p_button) == input::EKeyState::eHeld;
	}

	auto InputContext::_setKeyState(input::EKeyCode p_key_code, input::EKeyState p_key_state) -> void
	{
		m_keyStateMap[p_key_code] = p_key_state;
	}

	auto InputContext::_setMouseButtonState(input::EMouseButton p_mouse_button, input::EKeyState p_key_state) -> void
	{
		m_mouseButtonStateMap[p_mouse_button] = p_key_state;
	}

	auto InputContext::_setMouseScroll(float32 p_x_offset, float32 p_y_offset) -> void
	{
		m_mouseScroll = std::make_pair(p_x_offset, p_y_offset);
	}

	auto InputContext::_update() -> void
	{
		for (const auto &[key, state]: m_keyStateMap)
			if (state == input::EKeyState::ePressed)
				_setKeyState(key, input::EKeyState::eHeld);

		for (const auto &[key, state]: m_keyStateMap)
			if (state == input::EKeyState::eReleased)
				_setKeyState(key, input::EKeyState::eNone);

		for (const auto &[key, state]: m_mouseButtonStateMap)
			if (state == input::EKeyState::ePressed)
				_setMouseButtonState(key, input::EKeyState::eHeld);

		for (const auto &[key, state]: m_mouseButtonStateMap)
			if (state == input::EKeyState::eReleased)
				_setMouseButtonState(key, input::EKeyState::eNone);

		m_mouseScroll = {0.0f, 0.0f};
	}

	auto InputContext::_onEndFrame() -> void
	{
	}
}
