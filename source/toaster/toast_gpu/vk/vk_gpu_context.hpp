#pragma once

#include "../gpu_context.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"

#include <unordered_set>

namespace toaster::gpu
{
	#ifdef NDEBUG
	constexpr bool c_enableValidationLayers{false};
	#else
	constexpr bool c_enableValidationLayers{true};
	#endif

	class VKGPUContext final : public IGPUContext
	{
	public:
		VKGPUContext(GLFWwindow *p_window);
		~VKGPUContext() noexcept override;

		[[nodiscard]] vk::raii::Instance &      getVulkanInstance();
		[[nodiscard]] vk::raii::PhysicalDevice &getPhysicalDevice();

	private:
		void _createInstance();
		void _createDebugMessenger();
		void _createSurface();
		void _pickPhysicalDevice();
		void _createLogicalDevice();
		void _createSwapchain();

		bool _isDeviceSuitable(const vk::raii::PhysicalDevice &p_physical_device);

		std::vector<CString> _getRequiredInstanceExtensions();

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL _debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      p_message_severity,
															   vk::DebugUtilsMessageTypeFlagsEXT             p_message_type,
															   const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data);

		vk::raii::Context  m_context;
		vk::raii::Instance m_vulkanInstance{nullptr};

		vk::raii::DebugUtilsMessengerEXT m_debugUtilsMessenger{nullptr};

		GLFWwindow *         m_window{nullptr}; // Used as a reference to create the window surface
		vk::raii::SurfaceKHR m_surface{nullptr};

		vk::raii::PhysicalDevice m_currentPhysicalDevice{nullptr};

		std::vector<CString> m_requiredDeviceExtensions{vk::KHRSwapchainExtensionName};
		vk::raii::Device     m_device{nullptr};

		vk::raii::Queue m_graphicsQueue{nullptr};

		vk::SurfaceFormatKHR _chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_available_formats);
		vk::PresentModeKHR   _chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &p_available_present_modes);
		vk::Extent2D         _chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities);
	};
}
