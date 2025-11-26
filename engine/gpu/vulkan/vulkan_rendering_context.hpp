#pragma once

#include "gpu/rendering_context.hpp"

#include <vulkan/vulkan.h>

namespace tst::gpu
{
	class VulkanRenderingContext : public RenderingContext
	{
	public:
		VulkanRenderingContext();
		~VulkanRenderingContext() override;

		EError initialize() override;

		WindowSurfaceID createSurface(const void *platform_data) override;
		void            setSurfaceVsyncMode(WindowSurfaceID surface_id, EVsyncMode vsync_mode) override;
		EVsyncMode      getSurfaceVsyncMode(WindowSurfaceID surface_id) override;
		void            setSurfaceSize(WindowSurfaceID surface_id, uint32 width, uint32 height) override;
		void            getSurfaceWidth(WindowSurfaceID surface_id) override;
		void            getSurfaceHeight(WindowSurfaceID surface_id) override;
		void            destroySurface(WindowSurfaceID surface_id) override;

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL _debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT p_message_severity,
																	  VkDebugUtilsMessageTypeFlagsEXT p_message_type,
																	  const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data);
		static VKAPI_ATTR VkBool32 VKAPI_CALL _debugReportCallback(VkDebugReportFlagsEXT p_flags, VkDebugReportObjectTypeEXT p_object_type, uint64_t p_object,
																   size_t p_location, int32_t p_message_code, const char *p_layer_prefix, const char *p_message,
																   void *p_user_data);

		EError _initVulkanInstanceExtensions();
		EError _initVulkanInstance();
		EError _initVulkanDevices();

		struct DeviceQueueFamilies
		{
			std::vector<VkQueueFamilyProperties> properties;
		};

		VkInstance                       m_instance = VK_NULL_HANDLE;
		std::vector<VkPhysicalDevice>    m_physicalDevices;
		std::vector<DeviceQueueFamilies> m_queueFamilies;

		std::vector<String> m_requiredExtensions;
		std::vector<String> m_enabledExtensions;

		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT m_debugCallback  = VK_NULL_HANDLE;

		bool m_enableValidationLayers = true;
	};
}
