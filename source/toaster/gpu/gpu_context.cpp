#include "gpu_context.hpp"
#include "logging.hpp"

#include <set>
#include <sstream>
#include <GLFW/glfw3.h>

#include "toast_assert.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace toaster::gpu
{
	VkBool32 GPUContext::_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
										const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, [[maybe_unused]] void *                            pUserData)
	{
		if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LOG_WARN("Vulkan validation layer: {}", pCallbackData->pMessage);
		}
		else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			LOG_ERROR("Vulkan validation layer: {}", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator,
										  VkDebugUtilsMessengerEXT *pDebugMessenger)
	{
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
	{
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	GPUContext::GPUContext()
	{
		_createInstance();
		_setupDebug();
	}

	GPUContext::~GPUContext()
	{
		m_nvrhiDevice = nullptr;

		m_logicalDevice.destroy();

		destroyDebugUtilsMessengerEXT(m_vulkanInstance, m_debugMessenger, nullptr);

		m_vulkanInstance.destroy();
	}

	void GPUContext::init(vk::SurfaceKHR p_window_surface)
	{
		_pickPhysicalDevice(p_window_surface);
		_createLogicalDevice();
		_createNVRHIObjects();
	}

	vk::Instance GPUContext::getInstance() const
	{
		return m_vulkanInstance;
	}

	bool GPUContext::isValidationEnabled() const
	{
		return m_enableValidationLayers;
	}

	bool GPUContext::isValidationLayerSupported(const std::string &p_layer_name) const
	{
		auto available_layers = vk::enumerateInstanceLayerProperties();
		return std::ranges::any_of(available_layers, [p_layer_name](const auto &layer)
		{
			return layer.layerName == p_layer_name;
		});
	}

	const std::unordered_set<std::string> &GPUContext::getEnabledValidationLayers() const
	{
		return m_enabledValidationLayers;
	}

	bool GPUContext::isInstanceExtensionEnabled(const std::string &p_extension_name) const
	{
		return m_enabledInstanceExtensions.contains(p_extension_name);
	}

	bool GPUContext::isInstanceExtensionSupported(const std::string &p_extension_name) const
	{
		auto available_extensions = vk::enumerateInstanceExtensionProperties();
		return std::ranges::any_of(available_extensions, [p_extension_name](const auto &extension)
		{
			return extension.extensionName == p_extension_name;
		});
	}

	const std::unordered_set<std::string> &GPUContext::getEnabledInstanceExtensions() const
	{
		return m_enabledInstanceExtensions;
	}

	bool GPUContext::isDeviceExtensionEnabled(const std::string &p_extension_name) const
	{
		return m_enabledDeviceExtensions.contains(p_extension_name);
	}

	bool GPUContext::isDeviceExtensionSupported(const std::string &p_extension_name) const
	{
		auto available_extensions = m_physicalDevice.enumerateDeviceExtensionProperties(nullptr);
		return std::ranges::any_of(available_extensions, [p_extension_name](const auto &extension)
		{
			return extension.extensionName == p_extension_name;
		});
	}

	const std::unordered_set<std::string> &GPUContext::getEnabledDeviceExtensions() const
	{
		return m_enabledDeviceExtensions;
	}

	vk::PhysicalDevice GPUContext::getPhysicalDevice() const
	{
		return m_physicalDevice;
	}

	vk::PhysicalDeviceFeatures GPUContext::getPhysicalDeviceFeatures() const
	{
		return m_physicalDevice.getFeatures();
	}

	vk::PhysicalDeviceProperties GPUContext::getPhysicalDeviceProperties() const
	{
		return m_physicalDevice.getProperties();
	}

	const GPUContext::QueueFamilyIndices &GPUContext::getQueueFamilyIndices() const
	{
		return m_queueFamilyIndices;
	}

	uint32 GPUContext::findMemoryTypeIndex(uint32 p_type_filter, vk::MemoryPropertyFlags p_flags) const
	{
		vk::PhysicalDeviceMemoryProperties memory_properties = m_physicalDevice.getMemoryProperties();

		for (uint32 i = 0; i < memory_properties.memoryTypeCount; i++)
		{
			if ((1u << i) & p_type_filter && (memory_properties.memoryTypes[i].propertyFlags & p_flags) == p_flags)
			{
				return i;
			}
		}

		throw std::runtime_error("Failed to find memory type");

		return UINT32_MAX;
	}

	vk::Format GPUContext::findSupportedFormat(const std::vector<vk::Format> &p_candidates, vk::ImageTiling p_tiling, vk::FormatFeatureFlags p_features) const
	{
		for (auto &format: p_candidates)
		{
			vk::FormatProperties properties = m_physicalDevice.getFormatProperties(format);
			if (p_tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & p_features) == p_features)
			{
				return format;
			}
			else if (p_tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & p_features) == p_features)
			{
				return format;
			}
		}
		return vk::Format::eUndefined;
	}

	vk::Format GPUContext::findDepthFormat() const
	{
		return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal,
								   vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}

	bool GPUContext::hasStencilComponent(vk::Format p_format) const
	{
		return p_format == vk::Format::eD32SfloatS8Uint || p_format == vk::Format::eD24UnormS8Uint;
	}

	GPUContext::QueueFamilyIndices GPUContext::findQueueFamilies(vk::PhysicalDevice p_physical_device, vk::SurfaceKHR p_window_surface) const
	{
		QueueFamilyIndices indices;

		auto  queue_family_properties = p_physical_device.getQueueFamilyProperties();
		int32 i                       = 0;
		for (const auto &queue_family: queue_family_properties)
		{
			if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				indices.graphics = i;
			}
			if (queue_family.queueFlags & vk::QueueFlagBits::eTransfer)
			{
				indices.transfer = i;
			}
			if (queue_family.queueFlags & vk::QueueFlagBits::eCompute)
			{
				indices.compute = i;
			}
			if (p_physical_device.getSurfaceSupportKHR(i, p_window_surface))
			{
				indices.present = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}
		return indices;
	}

	bool GPUContext::checkDeviceExtensionSupport(vk::PhysicalDevice p_physical_device) const
	{
		std::unordered_set<std::string> requiredExtensions = m_enabledDeviceExtensions;
		for (const auto deviceExtensions = p_physical_device.enumerateDeviceExtensionProperties(); const auto &extension: deviceExtensions)
		{
			requiredExtensions.erase(std::string(extension.extensionName.data()));
		}

		// the device is missing one or more required extensions
		if (!requiredExtensions.empty())
		{
			LOG_ERROR("Missing required extensions:\n[");
			for (const auto &extension: requiredExtensions)
			{
				LOG_ERROR("\t{}", extension);
			}
			LOG_ERROR("]\n");

			return false;
		}

		return true;
	}

	GPUContext::SwapchainSupportDetails GPUContext::querySwapchainSupport(vk::PhysicalDevice p_physical_device, vk::SurfaceKHR p_window_surface) const
	{
		SwapchainSupportDetails details;

		details.capabilities = p_physical_device.getSurfaceCapabilitiesKHR(p_window_surface);
		details.formats      = p_physical_device.getSurfaceFormatsKHR(p_window_surface);
		details.presentModes = p_physical_device.getSurfacePresentModesKHR(p_window_surface);

		return details;
	}

	vk::SurfaceFormatKHR GPUContext::chooseSwapchainFormat(const std::vector<vk::SurfaceFormatKHR> &p_available_formats) const
	{
		for (const auto &format: p_available_formats)
		{
			LOG_INFO("Surface format found: - \t(Format)[{:>}]\t(Colour space)[{:>}]", vk::to_string(format.format), vk::to_string(format.colorSpace));
			if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return format;
			}
		}

		return p_available_formats[0];
	}

	vk::PresentModeKHR GPUContext::choosePresentMode(const std::vector<vk::PresentModeKHR> &p_available_present_modes) const
	{
		for (const auto &present_mode: p_available_present_modes)
		{
			if (present_mode == vk::PresentModeKHR::eMailbox)
			{
				return present_mode;
			}
		}
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D GPUContext::chooseExtent(const vk::SurfaceCapabilitiesKHR &p_capabilities, GLFWwindow *p_target_window) const
	{
		if (p_capabilities.currentExtent.width != std::numeric_limits<uint32>::max())
		{
			return p_capabilities.currentExtent;
		}

		int32 width  = 0;
		int32 height = 0;
		glfwGetFramebufferSize(p_target_window, &width, &height);

		vk::Extent2D extent{static_cast<uint32>(width), static_cast<uint32>(height)};

		extent.width  = std::clamp(extent.width, p_capabilities.minImageExtent.width, p_capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, p_capabilities.minImageExtent.height, p_capabilities.maxImageExtent.height);
		return extent;
	}

	vk::Device GPUContext::getLogicalDevice() const
	{
		return m_logicalDevice;
	}

	vk::Queue GPUContext::getGraphicsQueue() const
	{
		return m_graphicsQueue;
	}

	vk::Queue GPUContext::getPresentQueue() const
	{
		return m_presentQueue;
	}

	nvrhi::IDevice *GPUContext::getNVRHIDevice() const
	{
		return m_nvrhiDevice.Get();
	}

	nvrhi::Format GPUContext::getSwapchainFormat() const
	{
		return m_swapchainFormat;
	}

	void GPUContext::_createInstance()
	{
		vk::detail::DynamicLoader dl{};

		auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		if (m_enableValidationLayers && !_checkValidationLayerSupport())
		{
			throw std::runtime_error("Validation layers not available!");
		}

		// The info for our Vulkan application (technically not necessary, but it's nice to have)
		vk::ApplicationInfo app_info{};
		app_info.pApplicationName   = "ORBO";
		app_info.pEngineName        = "Toaster";
		app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion         = VK_MAKE_VERSION(1, 4, 0);

		vk::InstanceCreateInfo instance_create_info{};
		instance_create_info.pApplicationInfo = &app_info;

		uint32       glfw_extension_count     = 0u;
		const char **glfw_instance_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		LOG_INFO("GLFW required instance extensions:\n[");
		for (uint32 i = 0; i < glfw_extension_count; i++)
		{
			m_enabledInstanceExtensions.insert(std::string(glfw_instance_extensions[i]));

			LOG_INFO("\t{}", glfw_instance_extensions[i]);
		}
		LOG_INFO("]\n");

		std::unordered_set<std::string> requiredExtensions = m_enabledInstanceExtensions;

		auto available_instance_extensions = vk::enumerateInstanceExtensionProperties();

		LOG_INFO("Available instance extensions:\n[");
		for (const auto &instanceExt: available_instance_extensions)
		{
			const std::string name = instanceExt.extensionName;

			LOG_INFO("\t{}", name);
			if (m_optionalInstanceExtensions.contains(name))
			{
				m_enabledInstanceExtensions.insert(name);
			}

			requiredExtensions.erase(name);
		}
		LOG_INFO("]\n");

		if (!requiredExtensions.empty())
		{
			std::ostringstream ss;
			ss << "Cannot create a Vulkan instance because the following required extension(s) are not supported:";
			for (const auto &ext: requiredExtensions)
				ss << std::endl << "  - " << ext;

			LOG_ERROR("{}", ss.str());
			throw std::runtime_error(ss.str().c_str());
		}

		LOG_INFO("Enabled Vulkan instance extensions:\n[");
		for (const auto &ext: m_enabledInstanceExtensions)
		{
			LOG_INFO("\t{}", ext.c_str());
		}
		LOG_INFO("]\n");

		auto required_extensions       = stringSetToVector(m_enabledInstanceExtensions);
		auto enabled_validation_layers = stringSetToVector(m_enabledValidationLayers);

		instance_create_info.enabledExtensionCount   = required_extensions.size();
		instance_create_info.ppEnabledExtensionNames = required_extensions.data();

		if (m_enableValidationLayers)
		{
			instance_create_info.enabledLayerCount   = m_enabledValidationLayers.size();
			instance_create_info.ppEnabledLayerNames = enabled_validation_layers.data();
		}
		else
		{
			instance_create_info.enabledLayerCount = 0;
			instance_create_info.pNext             = nullptr;
		}

		m_vulkanInstance = vk::createInstance(instance_create_info);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_vulkanInstance);
	}

	void GPUContext::_setupDebug()
	{
		if (!m_enableValidationLayers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = _debugCallback;

		if (createDebugUtilsMessengerEXT(m_vulkanInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void GPUContext::_pickPhysicalDevice(vk::SurfaceKHR p_window_surface)
	{
		auto physical_devices = m_vulkanInstance.enumeratePhysicalDevices();

		for (const auto &device: physical_devices)
		{
			if (_isDeviceSuitable(device, p_window_surface))
			{
				m_physicalDevice = device;
				break;
			}
		}

		if (m_physicalDevice == nullptr)
		{
			throw std::runtime_error("Failed to find a suitable GPU!");
		}

		m_queueFamilyIndices = findQueueFamilies(m_physicalDevice, p_window_surface);
	}

	void GPUContext::_createLogicalDevice()
	{
		auto available_extensions = m_physicalDevice.enumerateDeviceExtensionProperties(nullptr);

		LOG_INFO("Available device extensions:\n[");
		for (const auto &extension: available_extensions)
		{
			const std::string name = extension.extensionName;
			LOG_INFO("\t{}", name);
			if (m_optionalDeviceExtensions.contains(name))
			{
				m_enabledDeviceExtensions.insert(name);
			}
		}
		LOG_INFO("]\n");

		bool timeline_semaphore_supported = false;
		bool mutable_format_supported     = false;

		LOG_INFO("Enabled device extensions: \n[");
		for (const auto &extension: m_enabledDeviceExtensions)
		{
			LOG_INFO("\t{}", extension);

			if (extension == VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)
				timeline_semaphore_supported = true;
			else if (extension == VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME)
				mutable_format_supported = true;
		}
		LOG_INFO("]\n");

		TST_ASSERT(timeline_semaphore_supported);

		#define APPEND_EXTENSION(condition, desc) if (condition) { (desc).pNext = pNext; pNext = &(desc); }
		void *pNext = nullptr;

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};
		std::set<int32>                        unique_queue_families = {m_queueFamilyIndices.graphics, m_queueFamilyIndices.present};
		constexpr float                        queue_priority        = 1.0f;

		for (int32 queue_family: unique_queue_families)
		{
			vk::DeviceQueueCreateInfo &queue_create_info = queue_create_infos.emplace_back();
			queue_create_info.queueFamilyIndex           = queue_family;
			queue_create_info.queueCount                 = 1;
			queue_create_info.pQueuePriorities           = &queue_priority;
		}

		vk::PhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features{};
		timeline_semaphore_features.timelineSemaphore = true;

		APPEND_EXTENSION(timeline_semaphore_supported, timeline_semaphore_features)

		vk::PhysicalDeviceFeatures device_features{};
		device_features.geometryShader            = true;
		device_features.shaderImageGatherExtended = true;
		device_features.samplerAnisotropy         = true;
		device_features.tessellationShader        = true;
		device_features.sampleRateShading         = true;
		device_features.wideLines                 = true;
		device_features.fillModeNonSolid          = true;

		auto                 enabled_extensions = stringSetToVector(m_enabledDeviceExtensions);
		vk::DeviceCreateInfo device_create_info{};
		device_create_info.pQueueCreateInfos       = queue_create_infos.data();
		device_create_info.queueCreateInfoCount    = queue_create_infos.size();
		device_create_info.pEnabledFeatures        = &device_features;
		device_create_info.enabledExtensionCount   = enabled_extensions.size();
		device_create_info.ppEnabledExtensionNames = enabled_extensions.data();
		device_create_info.pNext                   = pNext;

		m_logicalDevice = m_physicalDevice.createDevice(device_create_info);
		m_graphicsQueue = m_logicalDevice.getQueue(m_queueFamilyIndices.graphics, 0);
		m_presentQueue  = m_logicalDevice.getQueue(m_queueFamilyIndices.present, 0);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_logicalDevice);
	}

	void GPUContext::_createNVRHIObjects()
	{
		auto vec_instance_ext = stringSetToVector(m_enabledInstanceExtensions);
		auto vec_layers       = stringSetToVector(m_enabledValidationLayers);
		auto vec_device_ext   = stringSetToVector(m_enabledDeviceExtensions);

		nvrhi::vulkan::DeviceDesc device_desc{};

		device_desc.errorCB            = nullptr;
		device_desc.instance           = m_vulkanInstance;
		device_desc.physicalDevice     = m_physicalDevice;
		device_desc.device             = m_logicalDevice;
		device_desc.graphicsQueue      = m_graphicsQueue;
		device_desc.graphicsQueueIndex = m_queueFamilyIndices.graphics;

		device_desc.instanceExtensions           = vec_instance_ext.data();
		device_desc.numInstanceExtensions        = vec_instance_ext.size();
		device_desc.deviceExtensions             = vec_device_ext.data();
		device_desc.numDeviceExtensions          = vec_device_ext.size();
		device_desc.bufferDeviceAddressSupported = false;

		m_nvrhiDevice = nvrhi::vulkan::createDevice(device_desc);
	}

	bool GPUContext::_checkValidationLayerSupport() const
	{
		const auto available_layers = vk::enumerateInstanceLayerProperties();

		LOG_INFO("Available validation layers:\n[");
		for (const auto &layer_properties: available_layers)
		{
			LOG_INFO("\t{} -> {}", layer_properties.layerName.data(), layer_properties.description.data());
		}
		LOG_INFO("]\n");

		for (const auto &layer_name: m_enabledValidationLayers)
		{
			bool layer_found = false;

			for (const auto &layer_properties: available_layers)
			{
				if (std::strcmp(layer_properties.layerName.data(), layer_name.c_str()) == 0)
				{
					layer_found = true;
					break;
				}
			}

			if (!layer_found)
			{
				return false;
			}
		}
		return true;
	}

	bool GPUContext::_isDeviceSuitable(vk::PhysicalDevice p_physical_device, vk::SurfaceKHR p_window_surface) const
	{
		QueueFamilyIndices queue_family_indices = findQueueFamilies(p_physical_device, p_window_surface);

		bool extensions_supported = checkDeviceExtensionSupport(p_physical_device);
		bool swapchain_adequate   = false;
		if (extensions_supported)
		{
			SwapchainSupportDetails support_details = querySwapchainSupport(p_physical_device, p_window_surface);
			swapchain_adequate                      = !support_details.formats.empty() && !support_details.presentModes.empty();
		}

		return queue_family_indices.isComplete() && extensions_supported && swapchain_adequate;
	}
}
