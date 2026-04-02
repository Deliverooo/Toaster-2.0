#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include "../gpu_context.hpp"

#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"

#include <unordered_set>

#ifdef NDEBUG
constexpr bool c_enableValidationLayers{false};
#else
constexpr bool c_enableValidationLayers{true};
#endif

namespace toaster::gpu
{
	class VKGPUContext : public IGPUContext
	{
	public:
		VKGPUContext(GLFWwindow *p_window);
		~VKGPUContext() override;

		[[nodiscard]] vk::raii::Instance &getVulkanInstance();

		const std::vector<vk::raii::PhysicalDevice> &getPhysicalDevices() const;

	private:
		void                 _createInstance();
		void                 _createDebugMessenger();
		std::vector<CString> _getRequiredInstanceExtensions();

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL _debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      p_message_severity,
															   vk::DebugUtilsMessageTypeFlagsEXT             p_message_type,
															   const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data);

		vk::raii::Context  m_context;
		vk::raii::Instance m_vulkanInstance{nullptr};

		vk::raii::DebugUtilsMessengerEXT m_debugUtilsMessenger{nullptr};

		std::vector<vk::raii::PhysicalDevice> m_physicalDevices;
	};
}
