#include "toast_gpu/vk/vk_instance.hpp"

#include <vulkan/vulkan.hpp>

#include "toast_lib/os/terminal.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace toaster::gpu
{
	auto getDispatchLoader() -> vk::detail::DispatchLoaderDynamic
	{
		return vk::detail::defaultDispatchLoaderDynamic;
	}

	static VKAPI_ATTR auto VKAPI_CALL _debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      p_message_severity, vk::DebugUtilsMessageTypeFlagsEXT p_message_type,
													 const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data) -> vk::Bool32;

	VKInstance::VKInstance(const VKInstanceSpecInfo &p_spec_info) : m_specInfo(p_spec_info)
	{
		if (!m_specInfo.debugCallback)
			m_specInfo.debugCallback = &_debugCallback; // Fallback to the default one :)

		if (m_specInfo.enableValidationLayers)
		{
			m_specInfo.requiredExtensions.emplace(vk::EXTDebugUtilsExtensionName);
		}

		vk::ApplicationInfo app_info{};
		app_info.pApplicationName = "Toaster - Vulkan"; // The app and engine name for this can be completely arbitrary
		app_info.pEngineName      = "Toaster";
		// I want to use the latest vulkan version.
		//		 However I don't know if the vk::raii stuff will work with them or not
		app_info.apiVersion = vk::ApiVersion14;

		auto extension_props = m_context.enumerateInstanceExtensionProperties();

		if (m_specInfo.printDebugInfo)
		{
			LOG_INFO("Available instance extensions:");
			for (auto &prop: extension_props)
				LOG_INFO("\t{}", prop.extensionName.data());
			LOG_INFO("");
		}

		// Make sure that all the glfw extensions are present in the extension_props vector
		const auto unsupported_extension = std::ranges::find_if(m_specInfo.requiredExtensions, [extension_props](const auto &extension)
		{
			// returns true if none of the extensions are present (the strcmp would always evaluate to false)
			return std::ranges::none_of(extension_props, [ext = extension](const auto &prop)
			{
				return std::strcmp(prop.extensionName.data(), ext.c_str()) == 0;
			});
		});

		if (unsupported_extension != m_specInfo.requiredExtensions.end())
		{
			// We can't continue without the required glfw extensions, so terminate the program here
			LOG_ERROR("Required extension \"{}\" is not supported", *unsupported_extension);
			TST_PERMA_ASSERT(false);
		}
		std::vector<CString> required_validation_layers;
		if (m_specInfo.enableValidationLayers)
		{
			required_validation_layers.emplace_back("VK_LAYER_KHRONOS_validation");
			required_validation_layers.emplace_back("VK_LAYER_LUNARG_crash_diagnostic");

			auto output_path{os::getBinaryDirectory().string()};
			auto output_path_str{output_path.c_str()};

			_putenv_s("VK_LUNARG_CRASH_DIAGNOSTIC_OUTPUT_PATH", output_path_str);
			_putenv_s("VK_LUNARG_CRASH_DIAGNOSTIC_DUMP_SHADERS", "all");

			_putenv_s("VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES", "true");
			_putenv_s("VK_KHRONOS_VALIDATION_VALIDATE_BEST_PRACTICES_NVIDIA", "true");
		}

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
			TST_PERMA_ASSERT(false);
		}

		vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
		if (m_specInfo.enableValidationLayers)
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

		std::vector<CString> required_extensions_vec;
		for (const auto &ext: m_specInfo.requiredExtensions)
			required_extensions_vec.emplace_back(ext.c_str());
		vk::InstanceCreateInfo instance_create_info{};
		instance_create_info.pApplicationInfo        = &app_info;
		instance_create_info.enabledExtensionCount   = required_extensions_vec.size();
		instance_create_info.ppEnabledExtensionNames = required_extensions_vec.data();

		if (m_specInfo.enableValidationLayers)
		{
			instance_create_info.enabledLayerCount   = required_validation_layers.size();
			instance_create_info.ppEnabledLayerNames = required_validation_layers.data();
			instance_create_info.pNext               = &debug_messenger_create_info;
		}

		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
		m_vulkanInstance = vk::raii::Instance{m_context, instance_create_info};
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_vulkanInstance);
	}

	auto VKInstance::getSpecInfo() const -> const VKInstanceSpecInfo &
	{
		return m_specInfo;
	}

	auto VKInstance::getVulkanInstance() -> vk::raii::Instance &
	{
		return m_vulkanInstance;
	}

	auto VKInstance::initDispatcher(vk::Device p_device) const -> void
	{
		VULKAN_HPP_DEFAULT_DISPATCHER.init(p_device);
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
