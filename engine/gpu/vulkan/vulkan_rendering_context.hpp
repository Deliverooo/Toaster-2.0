#pragma once

#include "gpu/rendering_context.hpp"
#include "gpu/device.hpp"

#include <vulkan/vulkan.h>

namespace tst::gpu
{
	struct Surface
	{
		VkSurfaceKHR vulkanSurface = VK_NULL_HANDLE;
		uint32_t     width         = 0;
		uint32_t     height        = 0;
		EVsyncMode   vsyncMode     = EVsyncMode::eEnabled;
		bool         needsResize   = false;
	};

	class VulkanRenderingContext : public RenderingContext
	{
	public:
		VulkanRenderingContext() = default;
		~VulkanRenderingContext() override;

		EError initialize() override;

		WindowSurfaceID createSurface(const void *platform_data) override;
		void            setSurfaceVsyncMode(WindowSurfaceID surface_id, EVsyncMode vsync_mode) override;
		EVsyncMode      getSurfaceVsyncMode(WindowSurfaceID surface_id) override;
		void            setSurfaceSize(WindowSurfaceID surface_id, uint32 width, uint32 height) override;
		uint32          getSurfaceWidth(WindowSurfaceID surface_id) override;
		uint32          getSurfaceHeight(WindowSurfaceID surface_id) override;
		void            destroySurface(WindowSurfaceID surface_id) override;

		[[nodiscard]] const Device &getDevice(uint32 device_index) const override;
		[[nodiscard]] uint32        getDeviceCount() const override;
		[[nodiscard]] bool          deviceSupportsPresent(uint32 device_index, WindowSurfaceID surface_id) const override;

		[[nodiscard]] VkInstance              getVulkanInstance() const;
		[[nodiscard]] VkPhysicalDevice        getPhysicalDevice(uint32 device_index) const;
		[[nodiscard]] VkQueueFamilyProperties getQueueFamilyProperties(uint32 device_index, uint32 queue_family_index) const;
		bool                                  queueFamilySupportsPresent(VkPhysicalDevice physical_device, uint32 queue_family_index, WindowSurfaceID surface_id) const;

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

		VkInstance m_instance           = VK_NULL_HANDLE;
		uint32     m_instanceApiVersion = VK_API_VERSION_1_4;

		std::vector<String> m_requiredExtensions;
		std::vector<String> m_enabledExtensions;

		std::vector<Device>              m_devices;
		std::vector<VkPhysicalDevice>    m_physicalDevices;
		std::vector<DeviceQueueFamilies> m_queueFamilies;

		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT m_debugCallback  = VK_NULL_HANDLE;

		bool m_enableValidationLayers = true;



	};
}
