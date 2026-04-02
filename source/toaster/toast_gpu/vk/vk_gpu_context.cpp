#include "vk_gpu_context.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

namespace toaster::gpu
{
	VKGPUContext::VKGPUContext(GLFWwindow *p_window)
	{
		try
		{
			_createInstance();
			_createDebugMessenger();
		}
		catch (const vk::SystemError &err)
		{
			LOG_ERROR("Vulkan error: {}", err.what());
		}
		m_physicalDevices = m_vulkanInstance.enumeratePhysicalDevices();
	}

	VKGPUContext::~VKGPUContext()
	{
	}

	vk::raii::Instance &VKGPUContext::getVulkanInstance()
	{
		return m_vulkanInstance;
	}

	const std::vector<vk::raii::PhysicalDevice> &VKGPUContext::getPhysicalDevices() const
	{
		return m_physicalDevices;
	}

	void VKGPUContext::_createInstance()
	{
		vk::ApplicationInfo app_info{};
		app_info.pApplicationName = "Toaster"; // The app and engine name for this can be completely arbitrary
		app_info.pEngineName      = "Toaster";
		// I want to use the latest vulkan version.
		// TODO: Think about determining this beforehand to add support for older Vulkan versions.
		//		 However I don't know if the vk::raii stuff will work with them or not
		app_info.apiVersion = vk::ApiVersion14;

		auto required_extensions = _getRequiredInstanceExtensions();
		auto extension_props     = m_context.enumerateInstanceExtensionProperties();

		// Make sure that all the glfw extensions are present in the extension_props vector
		auto unsupported_extension = std::ranges::find_if(required_extensions, [extension_props](const auto &extension)
		{
			// returns true if none of the extensions are present (the strcmp would always evaluate to false)
			return std::ranges::none_of(extension_props, [ext = extension](const auto &prop)
			{
				return std::strcmp(prop.extensionName.data(), ext) == 0;
			});
		});

		if (unsupported_extension != required_extensions.end())
		{
			// We can't continue without the required glfw extensions, so terminate the program here
			LOG_ERROR("Required extension \"{}\" is not supported", *unsupported_extension);
			TST_ASSERT(false);
		}

		for (auto &prop: extension_props)
			LOG_INFO("Extension: {} | Version: {}", prop.extensionName.data(), prop.specVersion);

		std::vector<CString> required_validation_layers;
		if (c_enableValidationLayers)
			required_validation_layers.emplace_back("VK_LAYER_KHRONOS_validation");

		auto layer_props = m_context.enumerateInstanceLayerProperties();

		// Finds any layer in required_validation_layers, such that it is also not present in the actual layer_props vector
		auto unsupported_layer_it = std::ranges::find_if(required_validation_layers, [layer_props](const auto &layer)
		{
			return std::ranges::none_of(layer_props, [layer](const auto &prop)
			{
				return std::strcmp(prop.layerName.data(), layer) == 0;
			});
		});

		// Check to see if there are any unsupported layers
		if (unsupported_layer_it != required_validation_layers.end())
		{
			// We can't continue without the required validation layers, so terminate the program here
			LOG_ERROR("Found unsupported validation layer: {}", *unsupported_layer_it);
			TST_ASSERT(false);
		}

		vk::InstanceCreateInfo instance_create_info{};
		instance_create_info.pApplicationInfo        = &app_info;
		instance_create_info.enabledExtensionCount   = required_extensions.size();
		instance_create_info.ppEnabledExtensionNames = required_extensions.data();
		instance_create_info.flags                   = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

		m_vulkanInstance = vk::raii::Instance{m_context, instance_create_info};
	}

	void VKGPUContext::_createDebugMessenger()
	{
		vk::DebugUtilsMessageSeverityFlagsEXT severity_flags{
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
		};
		vk::DebugUtilsMessageTypeFlagsEXT message_type_flags{
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
		};
		vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
		debug_messenger_create_info.messageSeverity = severity_flags;
		debug_messenger_create_info.messageType     = message_type_flags;
		debug_messenger_create_info.pfnUserCallback = &_debugCallback;

		m_debugUtilsMessenger = m_vulkanInstance.createDebugUtilsMessengerEXT(debug_messenger_create_info);
	}

	std::vector<CString> VKGPUContext::_getRequiredInstanceExtensions()
	{
		// Gets all the possible platform-specific extension names that glfw needs to create a window.
		// For windows, one of them will be VK_KHR_win32_surface extension
		uint32 glfw_extension_count{0u};
		auto   glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		// Inserts the
		std::vector<CString> required_extensions{glfw_extension_count};
		for (uint32 i{0u}; i < glfw_extension_count; ++i)
			required_extensions[i] = glfw_extensions[i];

		required_extensions.emplace_back(vk::KHRPortabilityEnumerationExtensionName);

		if (c_enableValidationLayers)
			required_extensions.emplace_back(vk::EXTDebugUtilsExtensionName);

		return required_extensions;
	}

	vk::Bool32 VKGPUContext::_debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      p_message_severity, vk::DebugUtilsMessageTypeFlagsEXT p_message_type,
											const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data, void *                               p_user_data)
	{
		switch (p_message_severity)
		{
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			{
				LOG_TRACE("[Verbose] | Validation layer: {} | Message: {}", vk::to_string(p_message_type), p_callback_data->pMessage);
			}
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			{
				LOG_INFO("[Info] | Validation layer: {} | Message: {}", vk::to_string(p_message_type), p_callback_data->pMessage);
			}
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			{
				LOG_WARN("[Warning] | Validation layer: {} | Message: {}", vk::to_string(p_message_type), p_callback_data->pMessage);
			}
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			{
				LOG_ERROR("[Error] | Validation layer: {} | Message: {}", vk::to_string(p_message_type), p_callback_data->pMessage);
			}
		}
		return vk::False;
	}
}
