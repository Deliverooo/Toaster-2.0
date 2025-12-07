#include "device_manager.hpp"

#include "log.hpp"
#include "nvrhi/utils.h"
#include "platform/window.hpp"

#ifdef _WIN64
#include <ShellScalingApi.h>
#pragma comment(lib, "shcore.lib")
#endif

namespace tst
{
	// The joystick interface in glfw is not per-window like the keys, mouse, etc. The joystick callbacks
	// don't take a window arg. So glfw's model is a global joystick shared by all windows. Hence, the equivalent
	// is a singleton class that all DeviceManager instances can use.
	class JoyStickManager
	{
	public:
		static JoyStickManager &Singleton()
		{
			static JoyStickManager singleton;
			return singleton;
		}

		void UpdateAllJoysticks(const std::list<IRenderPass *> &passes);

		void EraseDisconnectedJoysticks();
		void EnumerateJoysticks();

		void ConnectJoystick(int id);
		void DisconnectJoystick(int id);

	private:
		JoyStickManager()
		{
		}

		void UpdateJoystick(int j, const std::list<IRenderPass *> &passes);

		std::list<int> m_JoystickIDs, m_RemovedJoysticks;
	};

	static void WindowIconifyCallback_GLFW(GLFWwindow *window, int iconified)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->WindowIconifyCallback(iconified);
		manager->GetWindowContext()->onWindowIconifyCallback(iconified);
	}

	static void TitlebarHitTestCallback_GLFW(GLFWwindow *window, int x, int y, int *hit)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->GetWindowContext()->onTitlebarHitTestCallback(x, y, hit);
	}

	static void WindowFocusCallback_GLFW(GLFWwindow *window, int focused)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->WindowFocusCallback(focused);
	}

	static void WindowRefreshCallback_GLFW(GLFWwindow *window)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->WindowRefreshCallback();
	}

	static void WindowCloseCallback_GLFW(GLFWwindow *window)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->WindowCloseCallback();
		manager->GetWindowContext()->onWindowCloseCallback();
	}

	static void WindowPosCallback_GLFW(GLFWwindow *window, int xpos, int ypos)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->WindowPosCallback(xpos, ypos);
	}

	static void KeyCallback_GLFW(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->KeyboardUpdate(key, scancode, action, mods);
		manager->GetWindowContext()->onKeyCallback(key, scancode, action, mods);
	}

	static void CharModsCallback_GLFW(GLFWwindow *window, unsigned int unicode, int mods)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->KeyboardCharInput(unicode, mods);
	}

	static void MousePosCallback_GLFW(GLFWwindow *window, double xpos, double ypos)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->MousePosUpdate(xpos, ypos);
		manager->GetWindowContext()->onMousePosCallback(xpos, ypos);
	}

	static void MouseButtonCallback_GLFW(GLFWwindow *window, int button, int action, int mods)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->MouseButtonUpdate(button, action, mods);
		manager->GetWindowContext()->onMouseButtonCallback(button, action, mods);
	}

	static void MouseScrollCallback_GLFW(GLFWwindow *window, double xoffset, double yoffset)
	{
		DeviceManager *manager = reinterpret_cast<DeviceManager *>(glfwGetWindowUserPointer(window));
		manager->MouseScrollUpdate(xoffset, yoffset);
		manager->GetWindowContext()->onMouseScrollCallback(xoffset, yoffset);
	}

	static void JoystickConnectionCallback_GLFW(int joyId, int connectDisconnect)
	{
		if (connectDisconnect == GLFW_CONNECTED)
			JoyStickManager::Singleton().ConnectJoystick(joyId);
		if (connectDisconnect == GLFW_DISCONNECTED)
			JoyStickManager::Singleton().DisconnectJoystick(joyId);
	}

	bool DeviceManager::CreateInstance(const InstanceParameters &params)
	{
		m_InstanceCreated = CreateInstanceInternal();
		return m_InstanceCreated;
	}

	bool DeviceManager::CreateHeadlessDevice(const DeviceCreationParameters &params)
	{
		m_DeviceParams                = params;
		m_DeviceParams.headlessDevice = true;

		if (!CreateInstance(m_DeviceParams))
			return false;

		return CreateDevice();
	}

	bool DeviceManager::CreateDevice(const DeviceCreationParameters &params, const char *windowTitle)
	{
		m_DeviceParams = params;

		if (!CreateInstance(m_DeviceParams))
			return false;

		if (!CreateDevice())
			return false;

		return true;
	}

	void DeviceManager::UpdateAverageFrameTime(double elapsedTime)
	{
		m_FrameTimeSum += elapsedTime;
		m_NumberOfAccumulatedFrames += 1;

		if (m_FrameTimeSum > m_AverageTimeUpdateInterval && m_NumberOfAccumulatedFrames > 0)
		{
			m_AverageFrameTime          = m_FrameTimeSum / double(m_NumberOfAccumulatedFrames);
			m_NumberOfAccumulatedFrames = 0;
			m_FrameTimeSum              = 0.0;
		}
	}

	void DeviceManager::AnimateRenderPresent()
	{
		#if 0
		double curTime = glfwGetTime(); double elapsedTime = curTime - m_PreviousFrameTimestamp; JoyStickManager::Singleton().EraseDisconnectedJoysticks();
		JoyStickManager::Singleton().UpdateAllJoysticks(m_vRenderPasses); if (m_windowVisible)
		{
			if (m_callbacks.beforeAnimate)
				m_callbacks.beforeAnimate(*this);
			Animate(elapsedTime);
			if (m_callbacks.afterAnimate)
				m_callbacks.afterAnimate(*this);
			if (m_callbacks.beforeRender)
				m_callbacks.beforeRender(*this);
			Render();
			if (m_callbacks.afterRender)
				m_callbacks.afterRender(*this);
			if (m_callbacks.beforePresent)
				m_callbacks.beforePresent(*this);
			Present();
			if (m_callbacks.afterPresent)
				m_callbacks.afterPresent(*this);
		} std::this_thread::sleep_for(std::chrono::milliseconds(0)); GetDevice()->runGarbageCollection(); UpdateAverageFrameTime(elapsedTime);
		m_PreviousFrameTimestamp = curTime; ++m_FrameIndex;
		#endif
	}

	void DeviceManager::GetWindowDimensions(int &width, int &height)
	{
		width  = m_DeviceParams.backBufferWidth;
		height = m_DeviceParams.backBufferHeight;
	}

	const DeviceCreationParameters &DeviceManager::GetDeviceParams()
	{
		return m_DeviceParams;
	}

	void DeviceManager::WindowPosCallback(int x, int y)
	{
		#ifdef _WINDOWS
		if (m_DeviceParams.enablePerMonitorDPI)
		{
			HWND hwnd    = glfwGetWin32Window(m_WindowHandle);
			auto monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

			unsigned int dpiX;
			unsigned int dpiY;
			GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);

			m_DPIScaleFactorX = dpiX / 96.f;
			m_DPIScaleFactorY = dpiY / 96.f;
		}
		#endif

		#if 0
		if (m_EnableRenderDuringWindowMovement && m_SwapChainFramebuffers.size() > 0)
		{
			if (m_callbacks.beforeFrame)
				m_callbacks.beforeFrame(*this);
			AnimateRenderPresent();
		}
		#endif
	}

	void DeviceManager::KeyboardUpdate(int key, int scancode, int action, int mods)
	{
		if (key == -1)
		{
			// filter unknown keys
			return;
		}

		for (auto it = m_vRenderPasses.crbegin(); it != m_vRenderPasses.crend(); it++)
		{
			bool ret = (*it)->KeyboardUpdate(key, scancode, action, mods);
			if (ret)
				break;
		}
	}

	void DeviceManager::KeyboardCharInput(unsigned int unicode, int mods)
	{
		for (auto it = m_vRenderPasses.crbegin(); it != m_vRenderPasses.crend(); it++)
		{
			bool ret = (*it)->KeyboardCharInput(unicode, mods);
			if (ret)
				break;
		}
	}

	void DeviceManager::MousePosUpdate(double xpos, double ypos)
	{
		xpos /= m_DPIScaleFactorX;
		ypos /= m_DPIScaleFactorY;

		#if 0
		for (auto it = m_vRenderPasses.crbegin(); it != m_vRenderPasses.crend(); it++)
		{
			bool ret = (*it)->MousePosUpdate(xpos, ypos);
			if (ret)
				break;
		}
		#endif
	}

	void DeviceManager::MouseButtonUpdate(int button, int action, int mods)
	{
		for (auto it = m_vRenderPasses.crbegin(); it != m_vRenderPasses.crend(); it++)
		{
			bool ret = (*it)->MouseButtonUpdate(button, action, mods);
			if (ret)
				break;
		}
	}

	void DeviceManager::MouseScrollUpdate(double xoffset, double yoffset)
	{
		for (auto it = m_vRenderPasses.crbegin(); it != m_vRenderPasses.crend(); it++)
		{
			bool ret = (*it)->MouseScrollUpdate(xoffset, yoffset);
			if (ret)
				break;
		}
	}

	void JoyStickManager::EnumerateJoysticks()
	{
		// The glfw header says nothing about what values to expect for joystick IDs. Empirically, having connected two
		// simultaneously, glfw just seems to number them starting at 0.
		for (int i = 0; i != 10; ++i)
			if (glfwJoystickPresent(i))
				m_JoystickIDs.push_back(i);
	}

	void JoyStickManager::EraseDisconnectedJoysticks()
	{
		while (!m_RemovedJoysticks.empty())
		{
			auto id = m_RemovedJoysticks.back();
			m_RemovedJoysticks.pop_back();

			auto it = std::find(m_JoystickIDs.begin(), m_JoystickIDs.end(), id);
			if (it != m_JoystickIDs.end())
				m_JoystickIDs.erase(it);
		}
	}

	void JoyStickManager::ConnectJoystick(int id)
	{
		m_JoystickIDs.push_back(id);
	}

	void JoyStickManager::DisconnectJoystick(int id)
	{
		// This fn can be called from inside glfwGetJoystickAxes below (similarly for buttons, I guess).
		// We can't call m_JoystickIDs.erase() here and now. Save them for later. Forunately, glfw docs
		// say that you can query a joystick ID that isn't present.
		m_RemovedJoysticks.push_back(id);
	}

	void JoyStickManager::UpdateAllJoysticks(const std::list<IRenderPass *> &passes)
	{
		for (auto j = m_JoystickIDs.begin(); j != m_JoystickIDs.end(); ++j)
			UpdateJoystick(*j, passes);
	}

	static void ApplyDeadZone(tsm::vec2f &v, const float deadZone = 0.1f)
	{
		v *= std::max(tsm::length(v) - deadZone, 0.f) / (1.f - deadZone);
	}

	void JoyStickManager::UpdateJoystick(int j, const std::list<IRenderPass *> &passes)
	{
		GLFWgamepadstate gamepadState;
		glfwGetGamepadState(j, &gamepadState);

		float *axisValues = gamepadState.axes;

		auto updateAxis = [&](int axis, float axisVal)
		{
			#if TODO
			for (auto it = passes.crbegin(); it != passes.crend(); it++)
			{
				bool ret = (*it)->JoystickAxisUpdate(axis, axisVal);
				if (ret)
					break;
			}
			#endif
		};

		{
			tsm::vec2f v(axisValues[GLFW_GAMEPAD_AXIS_LEFT_X], axisValues[GLFW_GAMEPAD_AXIS_LEFT_Y]);
			ApplyDeadZone(v);
			updateAxis(GLFW_GAMEPAD_AXIS_LEFT_X, v.x);
			updateAxis(GLFW_GAMEPAD_AXIS_LEFT_Y, v.y);
		}

		{
			tsm::vec2f v(axisValues[GLFW_GAMEPAD_AXIS_RIGHT_X], axisValues[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
			ApplyDeadZone(v);
			updateAxis(GLFW_GAMEPAD_AXIS_RIGHT_X, v.x);
			updateAxis(GLFW_GAMEPAD_AXIS_RIGHT_Y, v.y);
		}

		updateAxis(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, axisValues[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]);
		updateAxis(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, axisValues[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);

		for (int b = 0; b != GLFW_GAMEPAD_BUTTON_LAST; ++b)
		{
			auto buttonVal = gamepadState.buttons[b];
			for (auto it = passes.crbegin(); it != passes.crend(); it++)
			{
				bool ret = (*it)->JoystickButtonUpdate(b, buttonVal == GLFW_PRESS);
				if (ret)
					break;
			}
		}
	}

	void DeviceManager::Shutdown()
	{
		DestroyDevice();

		m_InstanceCreated = false;
	}

	void DeviceManager::SetInformativeWindowTitle(const char *applicationName, const char *extraInfo)
	{
		std::stringstream ss;
		ss << applicationName;
		ss << " (" << nvrhi::utils::GraphicsAPIToString(GetDevice()->getGraphicsAPI());

		if (m_DeviceParams.enableDebugRuntime)
		{
			if (GetGraphicsAPI() == nvrhi::GraphicsAPI::VULKAN)
				ss << ", VulkanValidationLayer";
			else
				ss << ", DebugRuntime";
		}

		if (m_DeviceParams.enableNvrhiValidationLayer)
		{
			ss << ", NvrhiValidationLayer";
		}

		ss << ")";

		double frameTime = GetAverageFrameTimeSeconds();
		if (frameTime > 0)
		{
			ss << " - " << std::setprecision(4) << (1.0 / frameTime) << " FPS ";
		}

		if (extraInfo)
			ss << extraInfo;
	}

	DeviceManager *DeviceManager::Create(nvrhi::GraphicsAPI api, GLFWwindow *windowHandle)
	{
		switch (api)
		{
				#if TST_ENABLE_DX11
			case nvrhi::GraphicsAPI::D3D11:
				return CreateD3D11();
				#endif
				#if TST_ENABLE_DX12
			case nvrhi::GraphicsAPI::D3D12:
				return CreateD3D12();
				#endif
				#if TST_ENABLE_VULKAN
			case nvrhi::GraphicsAPI::VULKAN:
				return CreateVK(windowHandle);
				#endif
			default: TST_ERROR("DeviceManager::Create: Unsupported Graphics API {0}", (uint8_t) api);
				return nullptr;
		}
	}

	DefaultMessageCallback &DefaultMessageCallback::GetInstance()
	{
		static DefaultMessageCallback Instance;
		return Instance;
	}

	void DefaultMessageCallback::message(nvrhi::MessageSeverity severity, const char *messageText)
	{
		switch (severity)
		{
			case nvrhi::MessageSeverity::Info: TST_INFO("{0}", messageText);
				break;
			case nvrhi::MessageSeverity::Warning: TST_WARN("{0}", messageText);
				break;
			case nvrhi::MessageSeverity::Error: TST_ERROR("{0}", messageText);
				break;
			case nvrhi::MessageSeverity::Fatal: TST_FATAL("{0}", messageText);
				break;
		}
	}
}
