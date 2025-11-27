#include "gpu/vulkan/vulkan_rendering_context.hpp"
#include "core/logging/log.hpp"

#ifdef TST_PLATFORM_WINDOWS
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

namespace tst::gpu
{
	#ifdef TST_PLATFORM_WINDOWS
	struct WindowPlatformData
	{
		HWND      window;
		HINSTANCE instance;
	};
	#endif

	PFN_vkGetPhysicalDeviceFeatures2   fpGetPhysicalDeviceFeatures2   = nullptr;
	PFN_vkGetPhysicalDeviceProperties2 fpGetPhysicalDeviceProperties2 = nullptr;

	// Device.
	PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = nullptr;

	// Surfaces.
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR      fpGetPhysicalDeviceSurfaceSupportKHR      = nullptr;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      fpGetPhysicalDeviceSurfaceFormatsKHR      = nullptr;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR = nullptr;

	// Debug utils.
	PFN_vkCreateDebugUtilsMessengerEXT  fpCreateDebugUtilsMessengerEXT  = nullptr;
	PFN_vkDestroyDebugUtilsMessengerEXT fpDestroyDebugUtilsMessengerEXT = nullptr;
	PFN_vkCmdBeginDebugUtilsLabelEXT    fpCmdBeginDebugUtilsLabelEXT    = nullptr;
	PFN_vkCmdEndDebugUtilsLabelEXT      fpCmdEndDebugUtilsLabelEXT      = nullptr;
	PFN_vkSetDebugUtilsObjectNameEXT    fpSetDebugUtilsObjectNameEXT    = nullptr;

	// Debug report.
	PFN_vkCreateDebugReportCallbackEXT  fpCreateDebugReportCallbackEXT  = nullptr;
	PFN_vkDebugReportMessageEXT         fpDebugReportMessageEXT         = nullptr;
	PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallbackEXT = nullptr;

	VulkanRenderingContext::~VulkanRenderingContext()
	{
		if (m_debugMessenger != VK_NULL_HANDLE && fpDestroyDebugUtilsMessengerEXT != VK_NULL_HANDLE)
		{
			fpDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, VK_NULL_HANDLE);
		}
		if (m_debugCallback != VK_NULL_HANDLE && fpDestroyDebugReportCallbackEXT != VK_NULL_HANDLE)
		{
			fpDestroyDebugReportCallbackEXT(m_instance, m_debugCallback, VK_NULL_HANDLE);
		}
		if (m_instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(m_instance, nullptr);
		}
	}

	EError VulkanRenderingContext::initialize()
	{
		EError return_error = EError::eOk;

		if (return_error = _initVulkanInstanceExtensions(); return_error != EError::eOk)
		{
			return return_error;
		}
		if (return_error = _initVulkanInstance(); return_error != EError::eOk)
		{
			return return_error;
		}

		if (return_error = _initVulkanDevices(); return_error != EError::eOk)
		{
			return return_error;
		}

		return return_error;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderingContext::_debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      p_message_severity,
																				   VkDebugUtilsMessageTypeFlagsEXT             p_message_type,
																				   const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data)
	{
		if (p_message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			TST_LOG_ERROR("Vulkan Debug Messenger [{}] : {}", p_callback_data->pMessageIdName, p_callback_data->pMessage);
		}
		else if (p_message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			TST_LOG_WARNING("Vulkan Debug Messenger [{}] : {}", p_callback_data->pMessageIdName, p_callback_data->pMessage);
		}
		else if (p_message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			TST_LOG_INFO("Vulkan Debug Messenger [{}] : {}", p_callback_data->pMessageIdName, p_callback_data->pMessage);
		}
		else
		{
			TST_LOG_INFO("Vulkan Debug Messenger [{}] : {}", p_callback_data->pMessageIdName, p_callback_data->pMessage);
		}
		return VK_FALSE;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderingContext::_debugReportCallback(VkDebugReportFlagsEXT p_flags, VkDebugReportObjectTypeEXT p_object_type,
																				uint64_t p_object, size_t p_location, int32_t p_message_code, const char *p_layer_prefix,
																				const char *p_message, void *p_user_data)
	{
		switch (p_flags)
		{
			case VK_DEBUG_REPORT_ERROR_BIT_EXT:
			case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT: TST_LOG_ERROR("Vulkan Debug Report [{}] Code {} : {}", p_layer_prefix, p_message_code, p_message);
				break;
			case VK_DEBUG_REPORT_WARNING_BIT_EXT:
			case VK_DEBUG_REPORT_INFORMATION_BIT_EXT: TST_LOG_WARNING("Vulkan Debug Report [{}] Code {} : {}", p_layer_prefix, p_message_code, p_message);
				break;
			case VK_DEBUG_REPORT_DEBUG_BIT_EXT: TST_LOG_INFO("Vulkan Debug Report [{}] Code {} : {}", p_layer_prefix, p_message_code, p_message);
				break;
			default: TST_LOG_INFO("Vulkan Debug Report [{}] Code {} : {}", p_layer_prefix, p_message_code, p_message);
				break;
		}
		return VK_FALSE;
	}

	EError VulkanRenderingContext::_initVulkanInstanceExtensions()
	{
		m_requiredExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
		if (m_enableValidationLayers)
		{
			m_requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			m_requiredExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
		m_requiredExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		uint32 instanceExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(instanceExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, availableExtensions.data());
		for (const auto &requiredExt: m_requiredExtensions)
		{
			bool found = false;
			for (const auto &ext: availableExtensions)
			{
				if (requiredExt == ext.extensionName)
				{
					found = true;
					break;
				}
			}
			if (found)
			{
				m_enabledExtensions.push_back(requiredExt);
			}
			else
			{
				TST_LOG_ERROR("VulkanRenderingContext::_initVulkanInstanceExtensions - Required Vulkan instance extension not found: {}", requiredExt);
				return EError::eDeviceError;
			}
		}
		return EError::eOk;
	}

	EError VulkanRenderingContext::_initVulkanInstance()
	{
		VkApplicationInfo app_info  = {};
		app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName   = "Toaster";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName        = "ToasterEngine";
		app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion         = m_instanceApiVersion;

		VkInstanceCreateInfo instance_info = {};
		instance_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_info.pApplicationInfo     = &app_info;
		std::vector<const char *> enabledExtensionNames;
		for (const auto &ext: m_enabledExtensions)
		{
			enabledExtensionNames.push_back(ext.c_str());
		}
		instance_info.enabledExtensionCount   = static_cast<uint32>(enabledExtensionNames.size());
		instance_info.ppEnabledExtensionNames = enabledExtensionNames.data();
		if (m_enableValidationLayers)
		{
			std::vector<const char *> validationLayerNames = {"VK_LAYER_KHRONOS_validation"};
			instance_info.enabledLayerCount                = static_cast<uint32>(validationLayerNames.size());
			instance_info.ppEnabledLayerNames              = validationLayerNames.data();
		}
		else
		{
			instance_info.enabledLayerCount   = 0;
			instance_info.ppEnabledLayerNames = nullptr;
		}

		VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = {};

		debug_messenger_create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_messenger_create_info.pNext           = nullptr;
		debug_messenger_create_info.flags           = 0;
		debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug_messenger_create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
													  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug_messenger_create_info.pfnUserCallback = _debugMessengerCallback;
		debug_messenger_create_info.pUserData       = this;
		instance_info.pNext                         = &debug_messenger_create_info;

		VkResult result = vkCreateInstance(&instance_info, nullptr, &m_instance);
		if (result != VK_SUCCESS)
		{
			TST_LOG_ERROR("VulkanRenderingContext::_initVulkanInstance - Failed to create Vulkan instance. VkResult: {}", static_cast<int32>(result));
			return EError::eDeviceError;
		}

		VkResult res = fpCreateDebugUtilsMessengerEXT(m_instance, &debug_messenger_create_info, nullptr, &m_debugMessenger);
		if (res != VK_SUCCESS)
		{
			TST_LOG_ERROR("Failed to create Vulkan debug messenger. VkResult: {}", static_cast<int32>(res));
		}

		return EError::eOk;
	}

	EError VulkanRenderingContext::_initVulkanDevices()
	{
		uint32_t physical_device_count = 0;
		VkResult err                   = vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr);
		if (err != VK_SUCCESS)
		{
			TST_LOG_CRITICAL("Failed to enumerate physical devices. VkResult: {}", static_cast<int32>(err));
			return EError::eDeviceError;
		}

		m_physicalDevices.resize(physical_device_count);
		m_queueFamilies.resize(physical_device_count);
		err = vkEnumeratePhysicalDevices(m_instance, &physical_device_count, m_physicalDevices.data());
		if (err != VK_SUCCESS)
		{
			TST_LOG_CRITICAL("Failed to enumerate physical devices. VkResult: {}", static_cast<int32>(err));
			return EError::eDeviceError;
		}

		for (uint32 i = 0; i < m_physicalDevices.size(); i++)
		{
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(m_physicalDevices[i], &props);

			uint32 queue_family_properties_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevices[i], &queue_family_properties_count, nullptr);

			if (queue_family_properties_count > 0)
			{
				m_queueFamilies[i].properties.resize(queue_family_properties_count);
				vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevices[i], &queue_family_properties_count, m_queueFamilies[i].properties.data());
			}
		}

		return EError::eOk;
	}

	WindowSurfaceID VulkanRenderingContext::createSurface(const void *platform_data)
	{
		const WindowPlatformData *wpd = (const WindowPlatformData *) (platform_data);

		#ifdef TST_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR create_info = {};
		create_info.sType                       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		create_info.hinstance                   = wpd->instance;
		create_info.hwnd                        = wpd->window;

		VkSurfaceKHR vulkan_surface = VK_NULL_HANDLE;
		VkResult     err            = vkCreateWin32SurfaceKHR(m_instance, &create_info, VK_NULL_HANDLE, &vulkan_surface);

		if (err != VK_SUCCESS)
		{
			return WindowSurfaceID();
		}

		#endif
		Surface *surface       = tnew Surface;
		surface->vulkanSurface = vulkan_surface;
		return reinterpret_cast<WindowSurfaceID>(surface);
	}

	void VulkanRenderingContext::destroySurface(WindowSurfaceID surface_id)
	{
		Surface *surface = reinterpret_cast<Surface *>(surface_id);
		vkDestroySurfaceKHR(m_instance, surface->vulkanSurface, VK_NULL_HANDLE);
		tdelete surface;
	}

	uint32 VulkanRenderingContext::getSurfaceHeight(WindowSurfaceID surface_id)
	{
		Surface *surface = reinterpret_cast<Surface *>(surface_id);
		return surface->height;
	}

	uint32 VulkanRenderingContext::getSurfaceWidth(WindowSurfaceID surface_id)
	{
		Surface *surface = reinterpret_cast<Surface *>(surface_id);
		return surface->width;
	}

	void VulkanRenderingContext::setSurfaceSize(WindowSurfaceID surface_id, uint32 width, uint32 height)
	{
		Surface *surface     = reinterpret_cast<Surface *>(surface_id);
		surface->width       = width;
		surface->height      = height;
		surface->needsResize = true;
	}

	EVsyncMode VulkanRenderingContext::getSurfaceVsyncMode(WindowSurfaceID surface_id)
	{
		Surface *surface = reinterpret_cast<Surface *>(surface_id);
		return surface->vsyncMode;
	}

	void VulkanRenderingContext::setSurfaceVsyncMode(WindowSurfaceID surface_id, EVsyncMode vsync_mode)
	{
		Surface *surface     = reinterpret_cast<Surface *>(surface_id);
		surface->vsyncMode   = vsync_mode;
		surface->needsResize = true;
	}

	VkInstance VulkanRenderingContext::getVulkanInstance() const
	{
		return m_instance;
	}

	bool VulkanRenderingContext::deviceSupportsPresent(uint32 device_index, WindowSurfaceID surface_id) const
	{
		TST_ASSERT(device_index < m_physicalDevices.size());

		const VkPhysicalDevice     physical_device = m_physicalDevices[device_index];
		const DeviceQueueFamilies &queue_families  = m_queueFamilies[device_index];
		for (uint32_t i = 0; i < queue_families.properties.size(); i++)
		{
			if ((queue_families.properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && queueFamilySupportsPresent(physical_device, i, surface_id))
			{
				return true;
			}
		}

		return false;
	}

	const Device &VulkanRenderingContext::getDevice(uint32 device_index) const
	{
		TST_ASSERT(device_index < m_devices.size());
		return m_devices[device_index];
	}

	uint32 VulkanRenderingContext::getDeviceCount() const
	{
		return static_cast<uint32>(m_devices.size());
	}

	VkPhysicalDevice VulkanRenderingContext::getPhysicalDevice(uint32 device_index) const
	{
		TST_ASSERT(device_index < m_physicalDevices.size());
		return m_physicalDevices[device_index];
	}

	VkQueueFamilyProperties VulkanRenderingContext::getQueueFamilyProperties(uint32 device_index, uint32 queue_family_index) const
	{
		TST_ASSERT(device_index < m_physicalDevices.size());
		TST_ASSERT(queue_family_index < m_queueFamilies.size());
		return m_queueFamilies[device_index].properties[queue_family_index];
	}

	bool VulkanRenderingContext::queueFamilySupportsPresent(VkPhysicalDevice physical_device, uint32 queue_family_index, WindowSurfaceID surface_id) const
	{
		TST_ASSERT(physical_device != VK_NULL_HANDLE)
		TST_ASSERT(surface_id != 0);
		Surface *surface                  = reinterpret_cast<Surface *>(surface_id);
		VkBool32 surface_supports_present = false;
		VkResult err                      = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, surface->vulkanSurface, &surface_supports_present);
		return err == VK_SUCCESS && surface_supports_present;
	}
}
