#include "vk_gpu_context.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	VKGPUContext::VKGPUContext(GLFWwindow *p_window) : m_window(p_window)
	{
		try
		{
			_createInstance();
			_createDebugMessenger();
			_createSurface();
			_pickPhysicalDevice();
			_createLogicalDevice();
			_createCommandPool();
		}
		catch (const vk::SystemError &err)
		{
			LOG_ERROR("Vulkan error: {}", err.what());
		}
	}

	VKGPUContext::~VKGPUContext() noexcept = default;

	vk::raii::Instance &VKGPUContext::getVulkanInstance()
	{
		return m_vulkanInstance;
	}

	vk::raii::PhysicalDevice &VKGPUContext::getPhysicalDevice()
	{
		return m_currentPhysicalDevice;
	}

	vk::raii::Device &VKGPUContext::getDevice()
	{
		return m_device;
	}

	vk::raii::Queue &VKGPUContext::getGraphicsQueue()
	{
		return m_graphicsQueue;
	}

	vk::raii::SurfaceKHR &VKGPUContext::getSurface()
	{
		return m_surface;
	}

	vk::raii::CommandPool &VKGPUContext::getCommandPool()
	{
		return m_commandPool;
	}

	void VKGPUContext::drawFrame()
	{
		// _recordCommandBuffer(image_index);
	}

	void VKGPUContext::_createInstance()
	{
		vk::ApplicationInfo app_info{};
		app_info.pApplicationName = "Toaster - Vulkan"; // The app and engine name for this can be completely arbitrary
		app_info.pEngineName      = "Toaster";
		// I want to use the latest vulkan version.
		// TODO: Think about determining this beforehand to add support for older Vulkan versions.
		//		 However I don't know if the vk::raii stuff will work with them or not
		app_info.apiVersion = vk::ApiVersion14;

		auto required_extensions = _getRequiredInstanceExtensions();
		auto extension_props     = m_context.enumerateInstanceExtensionProperties();

		// Make sure that all the glfw extensions are present in the extension_props vector
		const auto unsupported_extension = std::ranges::find_if(required_extensions, [extension_props](const auto &extension)
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

		vk::InstanceCreateInfo instance_create_info{};
		instance_create_info.pApplicationInfo        = &app_info;
		instance_create_info.enabledExtensionCount   = required_extensions.size();
		instance_create_info.ppEnabledExtensionNames = required_extensions.data();
		instance_create_info.flags                   = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

		m_vulkanInstance = vk::raii::Instance{m_context, instance_create_info};
	}

	void VKGPUContext::_createDebugMessenger()
	{
		constexpr vk::DebugUtilsMessageSeverityFlagsEXT severity_flags{
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
		};
		constexpr vk::DebugUtilsMessageTypeFlagsEXT message_type_flags{
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
		};
		vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
		debug_messenger_create_info.messageSeverity = severity_flags;
		debug_messenger_create_info.messageType     = message_type_flags;
		debug_messenger_create_info.pfnUserCallback = &_debugCallback;

		m_debugUtilsMessenger = m_vulkanInstance.createDebugUtilsMessengerEXT(debug_messenger_create_info);
	}

	void VKGPUContext::_createSurface()
	{
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(*m_vulkanInstance, m_window, nullptr, &surface) != VK_SUCCESS)
		{
			LOG_ERROR("Failed to create window surface");
			TST_ASSERT(false);
		}
		m_surface = {m_vulkanInstance, surface};
	}

	void VKGPUContext::_pickPhysicalDevice()
	{
		auto physical_devices = m_vulkanInstance.enumeratePhysicalDevices();
		if (physical_devices.empty())
		{
			// If your gpu does not have Vulkan support, we can't use Vulkan
			LOG_ERROR("Failed to find physical devices with Vulkan support");
			TST_ASSERT(false);
		}

		const auto device_it = std::ranges::find_if(physical_devices, [this](const auto &device)
		{
			return _isDeviceSuitable(device);
		});
		if (device_it == physical_devices.end())
		{
			LOG_ERROR("Failed to find suitable physical device");
			TST_ASSERT(false);
		}

		m_currentPhysicalDevice = *device_it;
	}

	void VKGPUContext::_createLogicalDevice()
	{
		auto queue_family_props = m_currentPhysicalDevice.getQueueFamilyProperties();

		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			if (queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics && m_currentPhysicalDevice.getSurfaceSupportKHR(i, m_surface))
			{
				m_queueFamilyIndices.graphics = i;
				break;
			}
		}
		if (m_queueFamilyIndices.graphics == UINT32_MAX)
		{
			LOG_ERROR("Failed to find a queue family that supports present");
			TST_ASSERT(false);
		}

		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> feature_chain{{}, {}, {}};
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering                    = true;
		feature_chain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;

		std::array<vk::DeviceQueueCreateInfo, 1> queue_create_infos{};
		// Create the graphics and present queue
		// The graphics queue should be the same as the present one
		queue_create_infos[0].queueFamilyIndex = m_queueFamilyIndices.graphics;
		queue_create_infos[0].queueCount       = 1;
		constexpr float32 queue_priority       = 1.0f;
		queue_create_infos[0].pQueuePriorities = &queue_priority;

		vk::DeviceCreateInfo device_create_info{};
		device_create_info.enabledExtensionCount   = static_cast<uint32>(m_requiredDeviceExtensions.size());
		device_create_info.ppEnabledExtensionNames = m_requiredDeviceExtensions.data();
		device_create_info.queueCreateInfoCount    = queue_create_infos.size();
		device_create_info.pQueueCreateInfos       = queue_create_infos.data();
		device_create_info.pNext                   = &feature_chain.get<vk::PhysicalDeviceFeatures2>();

		m_device        = {m_currentPhysicalDevice, device_create_info};
		m_graphicsQueue = {m_device, m_queueFamilyIndices.graphics, 0};
	}

	void VKGPUContext::_createCommandPool()
	{
		vk::CommandPoolCreateInfo command_pool_create_info{};
		command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.graphics;
		command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_commandPool = {m_device, command_pool_create_info};
	}

	void VKGPUContext::_recreateSwapchain()
	{
		m_device.waitIdle();
		//
		// m_swapchainImageViews.clear();
		// m_swapchain = nullptr;
		//
		// _createSwapchain();
		// _createImageViews();
	}

	bool VKGPUContext::_isDeviceSuitable(const vk::raii::PhysicalDevice &p_physical_device) const
	{
		auto props              = p_physical_device.getProperties();
		bool vulkan_1_3_support = props.apiVersion >= vk::ApiVersion13;

		auto queue_families    = p_physical_device.getQueueFamilyProperties();
		bool supports_graphics = std::ranges::any_of(queue_families, [](const auto &queue_family)
		{
			return !!(queue_family.queueFlags & vk::QueueFlagBits::eGraphics);
		});

		std::vector required_device_extensions{vk::KHRSwapchainExtensionName};

		auto available_device_extensions             = p_physical_device.enumerateDeviceExtensionProperties();
		bool supports_all_required_device_extensions = std::ranges::all_of(required_device_extensions, [available_device_extensions](const auto &required_ext)
		{
			return std::ranges::any_of(available_device_extensions, [&required_ext](const auto &available_ext)
			{
				return std::strcmp(available_ext.extensionName, required_ext) == 0;
			});
		});

		auto features = p_physical_device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

		bool supports_required_features = features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering && features.get<
											  vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

		return vulkan_1_3_support && supports_graphics && supports_all_required_device_extensions && supports_required_features;
	}

	std::vector<CString> VKGPUContext::_getRequiredInstanceExtensions() const
	{
		// Gets all the possible platform-specific extension names that glfw needs to create a window.
		// For windows, one of them will be VK_KHR_win32_surface extension
		uint32     glfw_extension_count{0u};
		const auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

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
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose: LOG_TRACE("[Verbose] | Validation layer: {} | Message: {}", vk::to_string(p_message_type),
																			   p_callback_data->pMessage);
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo: LOG_INFO("[Info] | Validation layer: {} | Message: {}", vk::to_string(p_message_type),
																		   p_callback_data->pMessage);
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: LOG_WARN("[Warning] | Validation layer: {} | Message: {}", vk::to_string(p_message_type),
																			  p_callback_data->pMessage);
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError: LOG_ERROR("[Error] | Validation layer: {} | Message: {}", vk::to_string(p_message_type),
																			 p_callback_data->pMessage);
		}
		return vk::False;
	}

	void VKGPUContext::transitionImageLayout(vk::raii::CommandBuffer &p_command_buffer, vk::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
											 vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
											 vk::PipelineStageFlags2  p_dst_stage_mask)
	{
		vk::ImageMemoryBarrier2 image_memory_barrier{};
		image_memory_barrier.oldLayout           = p_old_layout;
		image_memory_barrier.newLayout           = p_new_layout;
		image_memory_barrier.srcAccessMask       = p_src_access_mask;
		image_memory_barrier.dstAccessMask       = p_dst_access_mask;
		image_memory_barrier.srcStageMask        = p_src_stage_mask;
		image_memory_barrier.dstStageMask        = p_dst_stage_mask;
		image_memory_barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		image_memory_barrier.image               = p_image;
		image_memory_barrier.subresourceRange    = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		p_command_buffer.pipelineBarrier2(dependency_info);
	}

	vk::raii::ShaderModule VKGPUContext::createShaderModule(const std::vector<uint8> &p_code)
	{
		vk::ShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.codeSize = p_code.size();
		shader_module_create_info.pCode    = reinterpret_cast<const uint32 *>(p_code.data());

		return {m_device, shader_module_create_info};
	}
}
