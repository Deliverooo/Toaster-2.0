/*
*  	Copyright 2025 Orbo Stetson
 *  	Licensed under the Apache License, Version 2.0 (the "License");
 *  	you may not use this file except in compliance with the License.
 *  	You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */
#pragma once

#ifdef TST_ENABLE_DX11 || TST_ENABLE_DX12
#include <dxgi.h>
#endif

#ifdef TST_ENABLE_DX11
#include <d3d11.h>
#endif

#ifdef TST_ENABLE_DX12
#include <d3d12.h>
#endif

#ifdef TST_ENABLE_VULKAN
#include <vulkan/vulkan.hpp>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef TST_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3native.h>
#include <nvrhi/nvrhi.h>

#include <functional>
#include <list>

namespace tst
{
	struct DefaultMessageCallback : nvrhi::IMessageCallback
	{
		static DefaultMessageCallback &GetInstance();

		void message(nvrhi::MessageSeverity severity, const char *messageText) override;
	};

	struct InstanceParameters
	{
		bool enableDebugRuntime = false;
		bool headlessDevice     = false;

		#if TST_ENABLE_VULKAN
		std::vector<std::string> requiredVulkanInstanceExtensions;
		std::vector<std::string> requiredVulkanLayers;
		std::vector<std::string> optionalVulkanInstanceExtensions;
		std::vector<std::string> optionalVulkanLayers;
		#endif
	};

	struct DeviceCreationParameters : InstanceParameters
	{
		bool          startMaximized             = false; // ignores backbuffer width/height to be monitor size
		bool          startFullscreen            = false;
		bool          startBorderless            = false;
		bool          allowModeSwitch            = false;
		int           windowPosX                 = -1; // -1 means use default placement
		int           windowPosY                 = -1;
		uint32_t      backBufferWidth            = 1280;
		uint32_t      backBufferHeight           = 720;
		uint32_t      refreshRate                = 0;
		uint32_t      swapChainBufferCount       = 3;
		nvrhi::Format swapChainFormat            = nvrhi::Format::SRGBA8_UNORM;
		uint32_t      swapChainSampleCount       = 1;
		uint32_t      swapChainSampleQuality     = 0;
		uint32_t      maxFramesInFlight          = 2;
		bool          enableNvrhiValidationLayer = false;
		bool          vsyncEnabled               = false;
		bool          enableRayTracingExtensions = false; // for vulkan
		bool          enableComputeQueue         = false;
		bool          enableCopyQueue            = false;

		// Index of the adapter (DX11, DX12) or physical device (Vk) on which to initialize the device.
		// Negative values mean automatic detection.
		// The order of indices matches that returned by DeviceManager::EnumerateAdapters.
		int adapterIndex = -1;

		// set to true to enable DPI scale factors to be computed per monitor
		// this will keep the on-screen window size in pixels constant
		//
		// if set to false, the DPI scale factors will be constant but the system
		// may scale the contents of the window based on DPI
		//
		// note that the backbuffer size is never updated automatically; if the app
		// wishes to scale up rendering based on DPI, then it must set this to true
		// and respond to DPI scale factor changes by resizing the backbuffer explicitly
		bool enablePerMonitorDPI = false;

		#if TST_ENABLE_DX11 || TST_ENABLE_DX12
		DXGI_USAGE swapChainUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT; D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		#endif

		#if TST_ENABLE_VULKAN
		std::vector<std::string> requiredVulkanDeviceExtensions;
		std::vector<std::string> optionalVulkanDeviceExtensions;
		std::vector<size_t>      ignoredVulkanValidationMessageLocations = {
			// Ignore the warnings like "the storage image descriptor [...] is accessed by a OpTypeImage that has
			//   a Format operand ... which doesn't match the VkImageView ..." -- even when the GPU supports
			// storage without format, which all modern GPUs do, there is no good way to enable it in the shaders.
			0x13365b2
		};
		std::function<void(VkDeviceCreateInfo &)> deviceCreateInfoCallback;

		// This pointer specifies an optional structure to be put at the end of the chain for 'vkGetPhysicalDeviceFeatures2' call.
		// The structure may also be a chain, and must be alive during the device initialization process.
		// The elements of this structure will be populated before 'deviceCreateInfoCallback' is called,
		// thereby allowing applications to determine if certain features may be enabled on the device.
		void *physicalDeviceFeatures2Extensions = nullptr;
		#endif
	};

	struct AdapterInfo
	{
		using UUID = std::array<uint8_t, 16>;
		using LUID = std::array<uint8_t, 8>;

		std::string name;
		uint32_t    vendorID             = 0;
		uint32_t    deviceID             = 0;
		uint64_t    dedicatedVideoMemory = 0;

		std::optional<UUID> uuid;
		std::optional<LUID> luid;

		#if TST_ENABLE_DX11 || TST_ENABLE_DX12
		nvrhi::RefCountPtr<IDXGIAdapter> dxgiAdapter;
		#endif
		#if TST_ENABLE_VULKAN
		VkPhysicalDevice vkPhysicalDevice = nullptr;
		#endif
	};

	class IRenderPass;
	class Window;

	class DeviceManager
	{
	public:
		static DeviceManager *Create(nvrhi::GraphicsAPI api, GLFWwindow *windowHandle);

		bool         CreateHeadlessDevice(const DeviceCreationParameters &params);
		bool         CreateDevice(const DeviceCreationParameters &params, const char *windowTitle);
		virtual bool InitSurfaceCapabilities(uint64_t surfaceHandle) = 0;

		// Initializes device-independent objects (DXGI factory, Vulkan instnace).
		// Calling CreateInstance() is required before EnumerateAdapters(), but optional if you don't use EnumerateAdapters().
		// Note: if you call CreateInstance before Create*Device*(), the values in InstanceParameters must match those
		// in DeviceCreationParameters passed to the device call.
		bool CreateInstance(const InstanceParameters &params);

		// Enumerates adapters or physical devices present in the system.
		// Note: a call to CreateInstance() or Create*Device*() is required before EnumerateAdapters().
		virtual bool EnumerateAdapters(std::vector<AdapterInfo> &outAdapters) = 0;

		void RunMessageLoop();
		void AnimateRenderPresent();

		const DeviceCreationParameters &GetDeviceParams() const { return m_DeviceParams; }

		// returns the size of the window in screen coordinates
		void GetWindowDimensions(int &width, int &height);
		// returns the screen coordinate to pixel coordinate scale factor
		void GetDPIScaleInfo(float &x, float &y) const
		{
			x = m_DPIScaleFactorX;
			y = m_DPIScaleFactorY;
		}

		Window *GetWindowContext() { return m_window; }
		void    SetWindowContext(Window *window) { m_window = window; }

	protected:
		bool m_windowVisible = false;

		DeviceCreationParameters m_DeviceParams;

		Window *    m_window                           = nullptr;
		GLFWwindow *m_WindowHandle                     = nullptr;
		bool        m_EnableRenderDuringWindowMovement = false;
		// set to true if running on NV GPU
		bool                     m_IsNvidia = false;
		std::list<IRenderPass *> m_vRenderPasses;
		// timestamp in seconds for the previous frame
		double m_PreviousFrameTimestamp = 0.0;
		// current DPI scale info (updated when window moves)
		float m_DPIScaleFactorX = 1.f;
		float m_DPIScaleFactorY = 1.f;
		bool  m_RequestedVSync  = false;
		bool  m_InstanceCreated = false;

		double m_AverageFrameTime          = 0.0;
		double m_AverageTimeUpdateInterval = 0.5;
		double m_FrameTimeSum              = 0.0;
		int    m_NumberOfAccumulatedFrames = 0;

		uint32_t m_FrameIndex = 0;

		DeviceManager() = default;

		void UpdateAverageFrameTime(double elapsedTime);
		// device-specific methods
		virtual bool CreateInstanceInternal() = 0;
		virtual bool CreateDevice() = 0;
		virtual void DestroyDevice() = 0;

	public:
		[[nodiscard]] virtual nvrhi::IDevice *   GetDevice() const = 0;
		[[nodiscard]] virtual const char *       GetRendererString() const = 0;
		[[nodiscard]] virtual nvrhi::GraphicsAPI GetGraphicsAPI() const = 0;

		const DeviceCreationParameters &GetDeviceParams();
		[[nodiscard]] double            GetAverageFrameTimeSeconds() const { return m_AverageFrameTime; }
		[[nodiscard]] double            GetPreviousFrameTimestamp() const { return m_PreviousFrameTimestamp; }
		void                            SetFrameTimeUpdateInterval(double seconds) { m_AverageTimeUpdateInterval = seconds; }
		[[nodiscard]] bool              IsVsyncEnabled() const { return m_DeviceParams.vsyncEnabled; }

		virtual void SetVsyncEnabled(bool enabled)
		{
			m_RequestedVSync = enabled; /* will be processed later */
		}

		virtual void ReportLiveObjects()
		{
		}

		void SetEnableRenderDuringWindowMovement(bool val) { m_EnableRenderDuringWindowMovement = val; }

		// these are public in order to be called from the GLFW callback functions
		void WindowCloseCallback()
		{
		}

		void WindowIconifyCallback(int iconified)
		{
		}

		void WindowFocusCallback(int focused)
		{
		}

		void WindowRefreshCallback()
		{
		}

		void WindowPosCallback(int xpos, int ypos);

		void KeyboardUpdate(int key, int scancode, int action, int mods);
		void KeyboardCharInput(unsigned int unicode, int mods);
		void MousePosUpdate(double xpos, double ypos);
		void MouseButtonUpdate(int button, int action, int mods);
		void MouseScrollUpdate(double xoffset, double yoffset);

		[[nodiscard]] uint32_t GetFrameIndex() const { return m_FrameIndex; }

		virtual void Shutdown();
		virtual      ~DeviceManager() = default;

		void SetInformativeWindowTitle(const char *applicationName, const char *extraInfo = nullptr);

		virtual bool IsVulkanInstanceExtensionEnabled(const char *extensionName) const { return false; }
		virtual bool IsVulkanDeviceExtensionEnabled(const char *extensionName) const { return false; }
		virtual bool IsVulkanLayerEnabled(const char *layerName) const { return false; }

		virtual void GetEnabledVulkanInstanceExtensions(std::vector<std::string> &extensions) const
		{
		}

		virtual void GetEnabledVulkanDeviceExtensions(std::vector<std::string> &extensions) const
		{
		}

		virtual void GetEnabledVulkanLayers(std::vector<std::string> &layers) const
		{
		}

		struct PipelineCallbacks
		{
			std::function<void(DeviceManager &)> beforeFrame   = nullptr;
			std::function<void(DeviceManager &)> beforeAnimate = nullptr;
			std::function<void(DeviceManager &)> afterAnimate  = nullptr;
			std::function<void(DeviceManager &)> beforeRender  = nullptr;
			std::function<void(DeviceManager &)> afterRender   = nullptr;
			std::function<void(DeviceManager &)> beforePresent = nullptr;
			std::function<void(DeviceManager &)> afterPresent  = nullptr;
		} m_callbacks;

	private:
		static DeviceManager *CreateD3D11();
		static DeviceManager *CreateD3D12();
		static DeviceManager *CreateVK(GLFWwindow *windowHandle);

		std::string m_WindowTitle;
	};

	class IRenderPass
	{
	private:
		DeviceManager *m_DeviceManager;

	public:
		explicit IRenderPass(DeviceManager *deviceManager)
			: m_DeviceManager(deviceManager)
		{
		}

		virtual ~IRenderPass() = default;

		virtual void Render(nvrhi::IFramebuffer *framebuffer)
		{
		}

		virtual void Animate(float fElapsedTimeSeconds)
		{
		}

		virtual void BackBufferResizing()
		{
		}

		virtual void BackBufferResized(const uint32_t width, const uint32_t height, const uint32_t sampleCount)
		{
		}

		// all of these pass in GLFW constants as arguments
		// see http://www.glfw.org/docs/latest/input.html
		// return value is true if the event was consumed by this render pass, false if it should be passed on
		virtual bool KeyboardUpdate(int key, int scancode, int action, int mods) { return false; }
		virtual bool KeyboardCharInput(unsigned int unicode, int mods) { return false; }
		virtual bool MousePosUpdate(double xpos, double ypos) { return false; }
		virtual bool MouseScrollUpdate(double xoffset, double yoffset) { return false; }
		virtual bool MouseButtonUpdate(int button, int action, int mods) { return false; }
		virtual bool JoystickButtonUpdate(int button, bool pressed) { return false; }
		virtual bool JoystickAxisUpdate(int axis, float value) { return false; }

		[[nodiscard]] DeviceManager * GetDeviceManager() const { return m_DeviceManager; }
		[[nodiscard]] nvrhi::IDevice *GetDevice() const { return m_DeviceManager->GetDevice(); }
		[[nodiscard]] uint32_t        GetFrameIndex() const { return m_DeviceManager->GetFrameIndex(); }
	};
}
