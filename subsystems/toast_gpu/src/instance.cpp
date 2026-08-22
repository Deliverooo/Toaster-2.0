#include "toast_gpu/instance.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE namespace toaster::gpu
{
	static auto defaultDebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      p_message_severity, vk::DebugUtilsMessageTypeFlagsEXT p_message_type,
									 const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data, [[maybe_unused]] void *              p_user_data) -> vk::Bool32
	{
		switch (p_message_severity)
		{
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
				std::printf("[Verbose] | Validation layer: %s | Message: %s", vk::to_string(p_message_type).c_str(), p_callback_data->pMessage);
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
				std::printf("[Info] | Validation layer: %s | Message: %s", vk::to_string(p_message_type).c_str(), p_callback_data->pMessage);
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
				std::printf("[Warning] | Validation layer: %s | Message: %s", vk::to_string(p_message_type).c_str(), p_callback_data->pMessage);
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
				std::printf("[Error] | Validation layer: %s | Message: %s", vk::to_string(p_message_type).c_str(), p_callback_data->pMessage);
				break;
			default: break;
		}
		return vk::False;
	}

	vk::detail::DispatchLoaderDynamic FunctionDispatcher::s_dldy{};

	auto FunctionDispatcher::initBaseFunctions() -> void
	{
		s_dldy.init();
	}

	auto FunctionDispatcher::initInstanceFunctions(vk::Instance p_instance) -> void
	{
		s_dldy.init(p_instance);
	}

	auto FunctionDispatcher::initDeviceFunctions(vk::Device p_device) -> void
	{
		s_dldy.init(p_device);
	}

	auto FunctionDispatcher::get() -> const vk::detail::DispatchLoaderDynamic &
	{
		return s_dldy;
	}

	Instance::Instance(const InstanceDesc &p_desc)
	{
		auto required_extensions_vec{p_desc.requiredExtensions | std::ranges::to<std::vector>()};

		if (p_desc.enableValidationLayers)
			required_extensions_vec.emplace_back(vk::EXTDebugUtilsExtensionName);

		std::vector extension_props{vk::enumerateInstanceExtensionProperties()};

		// Make sure that all the required extensions are present in the extension_props vector
		const auto unsupported_extension{
			std::ranges::find_if(required_extensions_vec, [extension_props](const auto &extension)
			{
				// returns true if none of the extensions are present (the strcmp would always evaluate to false)
				return std::ranges::none_of(extension_props, [ext = extension](const auto &prop)
				{
					return std::strcmp(prop.extensionName.data(), ext) == 0;
				});
			})
		};

		if (unsupported_extension != required_extensions_vec.end())
		{
			// We can't continue without the required extensions, so terminate the program here
			std::printf("Required extension \"%s\" is not supported", *unsupported_extension);
			TST_PERMA_ASSERT(false);
		}

		std::vector<CString> required_validation_layers;
		if (p_desc.enableValidationLayers)
		{
			required_validation_layers.emplace_back("VK_LAYER_KHRONOS_validation");
		}

		std::vector layer_props{vk::enumerateInstanceLayerProperties()};

		// Finds any layer in required_validation_layers, such that it is also not present in the actual layer_props vector
		const auto unsupported_layer_it{
			std::ranges::find_if(required_validation_layers, [layer_props](const auto &layer)
			{
				return std::ranges::none_of(layer_props, [layer](const auto &prop)
				{
					return std::strcmp(prop.layerName.data(), layer) == 0;
				});
			})
		};

		// Check to see if there are any unsupported layers
		if (unsupported_layer_it != required_validation_layers.end())
		{
			// We can't continue without the required validation layers, so terminate the program here
			std::printf("Found unsupported validation layer: %s", *unsupported_layer_it);
			TST_PERMA_ASSERT(false);
		}

		vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
		if (p_desc.enableValidationLayers)
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
			debug_messenger_create_info.pfnUserCallback = &defaultDebugCallback;
		}

		vk::ApplicationInfo application_info{};
		application_info.pApplicationName   = p_desc.applicationName;
		application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.pEngineName        = "Toaster";
		application_info.engineVersion      = VK_MAKE_VERSION(2, 1, 0);
		application_info.apiVersion         = vk::ApiVersion14;

		vk::InstanceCreateInfo instance_create_info{};
		instance_create_info.pApplicationInfo        = &application_info;
		instance_create_info.enabledExtensionCount   = required_extensions_vec.size();
		instance_create_info.ppEnabledExtensionNames = required_extensions_vec.data();

		if (p_desc.enableValidationLayers)
		{
			instance_create_info.enabledLayerCount   = required_validation_layers.size();
			instance_create_info.ppEnabledLayerNames = required_validation_layers.data();
			instance_create_info.pNext               = &debug_messenger_create_info;
		}

		m_vulkanInstance = vk::createInstance(instance_create_info);
	}

	Instance::~Instance()
	{
		m_vulkanInstance.destroy();
	}
}
