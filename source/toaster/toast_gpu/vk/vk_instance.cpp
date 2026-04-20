#include "vk_instance.hpp"

namespace toaster::gpu
{
	static VKAPI_ATTR auto VKAPI_CALL _debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      p_message_severity, vk::DebugUtilsMessageTypeFlagsEXT p_message_type,
													 const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data) -> vk::Bool32;

	VKInstance::VKInstance()
	{
	}

	auto VKInstance::create() -> void
	{
		vk::ApplicationInfo app_info{};
		app_info.pApplicationName = "Toaster - Vulkan"; // The app and engine name for this can be completely arbitrary
		app_info.pEngineName      = "Toaster";
		// I want to use the latest vulkan version.
		// TODO: Think about determining this beforehand to add support for older Vulkan versions.
		//		 However I don't know if the vk::raii stuff will work with them or not
		app_info.apiVersion = vk::ApiVersion14;

		if (c_enableValidationLayers)
			m_requiredExtensions.emplace(vk::EXTDebugUtilsExtensionName);

		auto extension_props = m_context.enumerateInstanceExtensionProperties();

		// Make sure that all the glfw extensions are present in the extension_props vector
		const auto unsupported_extension = std::ranges::find_if(m_requiredExtensions, [extension_props](const auto &extension)
		{
			// returns true if none of the extensions are present (the strcmp would always evaluate to false)
			return std::ranges::none_of(extension_props, [ext = extension](const auto &prop)
			{
				return std::strcmp(prop.extensionName.data(), ext) == 0;
			});
		});

		if (unsupported_extension != m_requiredExtensions.end())
		{
			// We can't continue without the required glfw extensions, so terminate the program here
			LOG_ERROR("Required extension \"{}\" is not supported", *unsupported_extension);
			TST_ASSERT(false);
		}

		LOG_INFO("Available instance extensions:");
		for (auto &prop: extension_props)
			LOG_INFO("\t{}", prop.extensionName.data());
		LOG_INFO("");

		std::vector<CString> required_validation_layers;
		if (c_enableValidationLayers)
			required_validation_layers.emplace_back("VK_LAYER_KHRONOS_validation");

		auto layer_props = m_context.enumerateInstanceLayerProperties();

		// Finds any layer in required_validation_layers, such that it is also not present in the actual layer_props vector
		const auto unsupported_layer_it = std::ranges::find_if(required_validation_layers, [layer_props](const auto &layer)
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

		if (!m_debugCallback)
			m_debugCallback = &_debugCallback; // Fallback to the default one :)

		vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
		if (c_enableValidationLayers)
		{
			constexpr vk::DebugUtilsMessageSeverityFlagsEXT severity_flags{
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
			};
			constexpr vk::DebugUtilsMessageTypeFlagsEXT message_type_flags{
				vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
			};
			debug_messenger_create_info.messageSeverity = severity_flags;
			debug_messenger_create_info.messageType     = message_type_flags;
			debug_messenger_create_info.pfnUserCallback = &_debugCallback;
		}

		const std::vector<CString> required_extensions_vec{m_requiredExtensions.begin(), m_requiredExtensions.end()};
		vk::InstanceCreateInfo     instance_create_info{};
		instance_create_info.pApplicationInfo        = &app_info;
		instance_create_info.enabledExtensionCount   = required_extensions_vec.size();
		instance_create_info.ppEnabledExtensionNames = required_extensions_vec.data();

		if (c_enableValidationLayers)
		{
			instance_create_info.enabledLayerCount   = required_validation_layers.size();
			instance_create_info.ppEnabledLayerNames = required_validation_layers.data();
			instance_create_info.pNext               = &debug_messenger_create_info;
		}

		m_vulkanInstance = vk::raii::Instance{m_context, instance_create_info};
	}

	auto VKInstance::getVulkanInstance() -> vk::raii::Instance &
	{
		return m_vulkanInstance;
	}

	auto VKInstance::setRequiredExtensions(const std::unordered_set<CString> &p_extensions) -> void
	{
		m_requiredExtensions = p_extensions;
	}

	auto VKInstance::setDebugCallback(vk::PFN_DebugUtilsMessengerCallbackEXT p_callback) -> void
	{
		m_debugCallback = p_callback;
	}

	auto _debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      p_message_severity, vk::DebugUtilsMessageTypeFlagsEXT p_message_type,
						const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data, [[maybe_unused]] void *              p_user_data) -> vk::Bool32
	{
		switch (p_message_severity)
		{
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose: LOG_TRACE("[Verbose] | Validation layer: {} | Message: {}", vk::to_string(p_message_type),
																			   p_callback_data->pMessage);
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo: LOG_INFO("[Info] | Validation layer: {} | Message: {}", vk::to_string(p_message_type),
																		   p_callback_data->pMessage);
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: LOG_WARN("[Warning] | Validation layer: {} | Message: {}", vk::to_string(p_message_type),
																			  p_callback_data->pMessage);
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError: LOG_ERROR("[Error] | Validation layer: {} | Message: {}", vk::to_string(p_message_type),
																			 p_callback_data->pMessage);
				break;
			default: break;
		}
		return vk::False;
	}
}
