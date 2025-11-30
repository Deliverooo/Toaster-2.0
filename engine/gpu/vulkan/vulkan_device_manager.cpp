#include "vulkan_device_manager.hpp"

#include "error_macros.hpp"

#include <iterator>
#include <algorithm>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace tst::gpu
{
	DefaultMessageCallback &DefaultMessageCallback::get()
	{
		static DefaultMessageCallback s_instance;
		return s_instance;
	}

	void DefaultMessageCallback::message(nvrhi::MessageSeverity severity, const char *messageText)
	{
		switch (severity)
		{
			case nvrhi::MessageSeverity::Info: TST_INFO("{0}", messageText);
				break;
			case nvrhi::MessageSeverity::Warning: TST_WARN("{0}", messageText);
				break;
			case nvrhi::MessageSeverity::Error: TST_ERROR("{0}", messageText);
				break;
			case nvrhi::MessageSeverity::Fatal: TST_FATAL("{0}", messageText);
				break;
		}
	}

	VulkanDeviceManager::VulkanDeviceManager()
	{
	}

	VulkanDeviceManager::~VulkanDeviceManager()
	{
	}

	EError VulkanDeviceManager::createDevice(const DeviceSpecInfo &info)
	{
		m_specInfo = info;

		EError ret;
		if (ret = _createVulkanInstance(); ret != EError::eOk)
		{
			return ret;
		}
		if (ret = _createDevice(); ret != EError::eOk)
		{
			return ret;
		}

		return EError::eOk;
	}

	void VulkanDeviceManager::destroyDevice()
	{
		m_nvrhiDevice = nullptr;

		if (m_vulkanDevice)
		{
			m_vulkanDevice.destroy();
			m_vulkanDevice = nullptr;
		}

		if (m_debugReportCallback)
		{
			m_vulkanInstance.destroyDebugReportCallbackEXT(m_debugReportCallback);
		}

		if (m_vulkanInstance)
		{
			m_vulkanInstance.destroy();
			m_vulkanInstance = nullptr;
		}
	}

	EError VulkanDeviceManager::_createDevice()
	{
		vk::DebugReportCallbackCreateInfoEXT debug_report_info{};
		debug_report_info.flags = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning;
		debug_report_info.setPfnCallback(_vulkanDebugCallback);
		debug_report_info.pUserData = this;

		vk::Result res = m_vulkanInstance.createDebugReportCallbackEXT(&debug_report_info, nullptr, &m_debugReportCallback);
		TST_ASSERT(res == vk::Result::eSuccess);

		if (m_specInfo.swapchainFormat == nvrhi::Format::SRGBA8_UNORM)
		{
			m_specInfo.swapchainFormat = nvrhi::Format::SBGRA8_UNORM;
		}
		else if (m_specInfo.swapchainFormat == nvrhi::Format::RGBA8_UNORM)
		{
			m_specInfo.swapchainFormat = nvrhi::Format::BGRA8_UNORM;
		}

		EError return_error;
		if (return_error = _pickPhysicalDevice(); return_error != EError::eOk)
		{
			return return_error;
		}

		if (!_findQueueFamilies(m_vulkanPhysicalDevice))
		{
			return EError::eDeviceError;
		}

		if (return_error = _createVulkanDevice(); return_error != EError::eOk)
		{
			return return_error;
		}

		std::vector<const char *> enabled_instance_extensions;
		enabled_instance_extensions.reserve(m_enabledInstanceExtensions.size());
		std::ranges::transform(m_enabledInstanceExtensions, std::back_inserter(enabled_instance_extensions), [](const String &str)
		{
			return str.c_str();
		});

		std::vector<const char *> enabled_validation_layers;
		enabled_validation_layers.reserve(m_enabledValidationLayers.size());
		std::ranges::transform(m_enabledValidationLayers, std::back_inserter(enabled_validation_layers), [](const String &str)
		{
			return str.c_str();
		});

		std::vector<const char *> enabled_device_extensions;
		enabled_device_extensions.reserve(m_enabledDeviceExtensions.size());
		std::ranges::transform(m_enabledDeviceExtensions, std::back_inserter(enabled_device_extensions), [](const String &str)
		{
			return str.c_str();
		});

		nvrhi::vulkan::DeviceDesc deviceDesc;
		deviceDesc.errorCB            = &DefaultMessageCallback::get();
		deviceDesc.instance           = m_vulkanInstance;
		deviceDesc.physicalDevice     = m_vulkanPhysicalDevice;
		deviceDesc.device             = m_vulkanDevice;
		deviceDesc.graphicsQueue      = m_graphicsQueue;
		deviceDesc.graphicsQueueIndex = m_queueFamilyIndices.graphics;

		deviceDesc.computeQueue      = m_computeQueue;
		deviceDesc.computeQueueIndex = m_queueFamilyIndices.compute;

		deviceDesc.transferQueue      = m_transferQueue;
		deviceDesc.transferQueueIndex = m_queueFamilyIndices.transfer;

		deviceDesc.instanceExtensions    = enabled_instance_extensions.data();
		deviceDesc.numInstanceExtensions = enabled_instance_extensions.size();

		deviceDesc.deviceExtensions    = enabled_device_extensions.data();
		deviceDesc.numDeviceExtensions = enabled_device_extensions.size();

		deviceDesc.bufferDeviceAddressSupported = m_bufferDeviceAddressSupported;

		m_nvrhiDevice = nvrhi::vulkan::createDevice(deviceDesc);

		return EError::eOk;
	}

	EError VulkanDeviceManager::_createVulkanInstance()
	{
		m_enabledInstanceExtensions.insert("VK_EXT_debug_report");
		m_enabledValidationLayers.insert("VK_LAYER_KHRONOS_validation");

		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = m_dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		if (!glfwVulkanSupported())
		{
			TST_ERROR("GLFW allegedly says that Vulkan is not supported!");
			return EError::eFailedToCreate;
		}

		#pragma region instance extensions
		uint32       glfwExtCount;
		const char **glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
		TST_ASSERT(glfwExt);

		for (uint32 i = 0u; i < glfwExtCount; i++)
		{
			m_enabledInstanceExtensions.insert(String(glfwExt[i]));
		}

		std::unordered_set<String> requiredInstanceExtensions = m_enabledInstanceExtensions;

		for (const auto &instance_ext: vk::enumerateInstanceExtensionProperties())
		{
			const String ext_name = instance_ext.extensionName;

			if (m_optionalInstanceExtensions.contains(ext_name))
			{
				m_enabledInstanceExtensions.insert(ext_name);
			}

			requiredInstanceExtensions.erase(ext_name);
		}

		if (!requiredInstanceExtensions.empty())
		{
			std::ostringstream oss;
			oss << "Failed to create a Vulkan instance. The following extensions are not supported:";
			for (const auto &ext: requiredInstanceExtensions)
			{
				oss << "\n-\t" << ext;
			}
			TST_ERROR("{}", oss.str());
			return EError::eFailedToCreate;
		}

		TST_INFO("Current enabled Vulkan instance extensions:");
		for (const auto &ext: m_enabledInstanceExtensions)
		{
			TST_INFO("\t-{}", ext);
		}
		#pragma endregion

		#pragma region validation layers

		std::unordered_set<String> requiredValidationLayers = m_enabledValidationLayers;
		for (const auto &validation_layer: vk::enumerateInstanceLayerProperties())
		{
			const String layer_name = validation_layer.layerName;

			if (m_optionalValidationLayers.contains(layer_name))
			{
				m_enabledValidationLayers.insert(layer_name);
			}

			requiredValidationLayers.erase(layer_name);
		}

		if (!requiredValidationLayers.empty())
		{
			std::ostringstream oss;
			oss << "Failed to create a Vulkan instance. The following validation layers are not supported:";
			for (const auto &ext: requiredValidationLayers)
			{
				oss << "\n-\t" << ext;
			}
			TST_ERROR("{}", oss.str());
			return EError::eFailedToCreate;
		}

		TST_INFO("Current enabled Vulkan validation layers:");
		for (const auto &vl: m_enabledValidationLayers)
		{
			TST_INFO("\t-{}", vl);
		}
		#pragma endregion

		vk::ApplicationInfo app_info{};
		vk::Result          res = vk::enumerateInstanceVersion(&app_info.apiVersion);

		if (res != vk::Result::eSuccess)
		{
			TST_ERROR("{}", vk::to_string(res));
			return EError::eFailedToCreate;
		}

		const uint32 minimum_vulkan_api_version = VK_MAKE_API_VERSION(0, 1, 4, 0);
		if (app_info.apiVersion < minimum_vulkan_api_version)
		{
			TST_ERROR("The current Vulkan API version on your system ({}) is too low.\n" " Please update / install a Vulkan SDK of at lease version ({})",
					  VK_API_VERSION_MAJOR(app_info.apiVersion), VK_API_VERSION_MINOR(app_info.apiVersion), VK_API_VERSION_PATCH(app_info.apiVersion),
					  VK_API_VERSION_MAJOR(minimum_vulkan_api_version), VK_API_VERSION_MINOR(minimum_vulkan_api_version),
					  VK_API_VERSION_PATCH(minimum_vulkan_api_version));
			return EError::eFailedToCreate;
		}

		std::vector<const char *> enabled_instance_extensions;
		enabled_instance_extensions.reserve(m_enabledInstanceExtensions.size());
		std::ranges::transform(m_enabledInstanceExtensions, std::back_inserter(enabled_instance_extensions), [](const String &str)
		{
			return str.c_str();
		});
		std::vector<const char *> enabled_validation_layers;
		enabled_validation_layers.reserve(m_enabledValidationLayers.size());
		std::ranges::transform(m_enabledValidationLayers, std::back_inserter(enabled_validation_layers), [](const String &str)
		{
			return str.c_str();
		});

		vk::InstanceCreateInfo instance_info{};
		instance_info.setEnabledLayerCount(enabled_validation_layers.size());
		instance_info.setPpEnabledLayerNames(enabled_validation_layers.data());
		instance_info.setEnabledExtensionCount(enabled_instance_extensions.size());
		instance_info.setPpEnabledExtensionNames(enabled_instance_extensions.data());
		instance_info.setPApplicationInfo(&app_info);

		res = vk::createInstance(&instance_info, nullptr, &m_vulkanInstance);
		if (res != vk::Result::eSuccess)
		{
			TST_ERROR("Failed to create Vulkan instance: {}", vk::to_string(res));
			return EError::eFailedToCreate;
		}

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_vulkanInstance);

		return EError::eOk;
	}

	EError VulkanDeviceManager::_createVulkanDevice()
	{
		auto device_extensions = m_vulkanPhysicalDevice.enumerateDeviceExtensionProperties();
		for (const auto &ext: device_extensions)
		{
			const String name = ext.extensionName;
			if (m_optionalDeviceExtensions.contains(name))
			{
				m_enabledDeviceExtensions.insert(name);
			}
		}
		m_enabledDeviceExtensions.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		const vk::PhysicalDeviceProperties physical_device_props = m_vulkanPhysicalDevice.getProperties();

		bool accelStructSupported      = false;
		bool rayPipelineSupported      = false;
		bool rayQuerySupported         = false;
		bool meshletsSupported         = false;
		bool vrsSupported              = false;
		bool synchronization2Supported = false;
		bool maintenance4Supported     = false;

		TST_INFO("Enabled Vulkan device extensions:");
		for (const auto &ext: m_enabledDeviceExtensions)
		{
			TST_INFO("\t-{}", ext.c_str());

			if (ext == VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
				accelStructSupported = true;
			else if (ext == VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
				rayPipelineSupported = true;
			else if (ext == VK_KHR_RAY_QUERY_EXTENSION_NAME)
				rayQuerySupported = true;
			else if (ext == VK_NV_MESH_SHADER_EXTENSION_NAME)
				meshletsSupported = true;
			else if (ext == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
				vrsSupported = true;
			else if (ext == VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
				synchronization2Supported = true;
			else if (ext == VK_KHR_MAINTENANCE_4_EXTENSION_NAME)
				maintenance4Supported = true;
		}

		#define APPEND_EXTENSION(condition, desc) if (condition) { (desc).pNext = pNext; pNext = &(desc); }

		void *pNext = nullptr;

		vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2;

		auto bufferDeviceAddressFeatures = vk::PhysicalDeviceBufferDeviceAddressFeatures();

		auto maintenance4Features = vk::PhysicalDeviceMaintenance4Features();

		APPEND_EXTENSION(true, bufferDeviceAddressFeatures);
		APPEND_EXTENSION(maintenance4Supported, maintenance4Features);

		physicalDeviceFeatures2.pNext = pNext;
		m_vulkanPhysicalDevice.getFeatures2(&physicalDeviceFeatures2);

		std::unordered_set<int> unique_queue_families = {m_queueFamilyIndices.graphics};

		unique_queue_families.insert(m_queueFamilyIndices.present);
		unique_queue_families.insert(m_queueFamilyIndices.compute);
		unique_queue_families.insert(m_queueFamilyIndices.transfer);

		float priority = 1.0f;

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
		queue_create_infos.reserve(unique_queue_families.size());
		for (int queue_family: unique_queue_families)
		{
			queue_create_infos.push_back(vk::DeviceQueueCreateInfo().setQueueFamilyIndex(queue_family).setQueueCount(1).setPQueuePriorities(&priority));
		}

		vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures{};
		accelStructFeatures.accelerationStructure = true;

		vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayPipelineFeatures{};
		rayPipelineFeatures.rayTracingPipeline           = true;
		rayPipelineFeatures.rayTraversalPrimitiveCulling = true;

		vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
		rayQueryFeatures.rayQuery = true;

		vk::PhysicalDeviceMeshShaderFeaturesNV meshletFeatures{};
		meshletFeatures.taskShader = true;
		meshletFeatures.meshShader = true;

		vk::PhysicalDeviceFragmentShadingRateFeaturesKHR vrsFeatures{};
		vrsFeatures.pipelineFragmentShadingRate   = true;
		vrsFeatures.primitiveFragmentShadingRate  = true;
		vrsFeatures.attachmentFragmentShadingRate = true;

		vk::PhysicalDeviceVulkan13Features vulkan13features{};
		vulkan13features.synchronization2 = synchronization2Supported;
		vulkan13features.maintenance4     = maintenance4Features.maintenance4;

		pNext = nullptr;
		APPEND_EXTENSION(accelStructSupported, accelStructFeatures)
		APPEND_EXTENSION(rayPipelineSupported, rayPipelineFeatures)
		APPEND_EXTENSION(rayQuerySupported, rayQueryFeatures)
		APPEND_EXTENSION(meshletsSupported, meshletFeatures)
		APPEND_EXTENSION(vrsSupported, vrsFeatures)
		APPEND_EXTENSION(physical_device_props.apiVersion >= VK_API_VERSION_1_3, vulkan13features)
		APPEND_EXTENSION(physical_device_props.apiVersion < VK_API_VERSION_1_3 && maintenance4Supported, maintenance4Features);
		#undef APPEND_EXTENSION

		vk::PhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.shaderImageGatherExtended = true;
		deviceFeatures.samplerAnisotropy         = true;
		deviceFeatures.tessellationShader        = true;
		deviceFeatures.textureCompressionBC      = true;
		deviceFeatures.geometryShader            = true;
		deviceFeatures.imageCubeArray            = true;
		deviceFeatures.dualSrcBlend              = true;
		deviceFeatures.independentBlend          = true;
		deviceFeatures.fillModeNonSolid          = true;
		deviceFeatures.wideLines                 = true;

		vk::PhysicalDeviceVulkan12Features vulkan12features{};
		vulkan12features.descriptorIndexing                        = true;
		vulkan12features.runtimeDescriptorArray                    = true;
		vulkan12features.descriptorBindingPartiallyBound           = true;
		vulkan12features.descriptorBindingVariableDescriptorCount  = true;
		vulkan12features.timelineSemaphore                         = true;
		vulkan12features.shaderSampledImageArrayNonUniformIndexing = true;
		vulkan12features.bufferDeviceAddress                       = bufferDeviceAddressFeatures.bufferDeviceAddress;
		vulkan12features.pNext                                     = pNext;

		std::vector<const char *> enabled_device_extensions;
		enabled_device_extensions.reserve(m_enabledDeviceExtensions.size());
		std::ranges::transform(m_enabledDeviceExtensions, std::back_inserter(enabled_device_extensions), [](const String &str)
		{
			return str.c_str();
		});
		std::vector<const char *> enabled_validation_layers;
		enabled_validation_layers.reserve(m_enabledValidationLayers.size());
		std::ranges::transform(m_enabledValidationLayers, std::back_inserter(enabled_validation_layers), [](const String &str)
		{
			return str.c_str();
		});

		vk::DeviceCreateInfo deviceDesc{};
		deviceDesc.pQueueCreateInfos       = queue_create_infos.data();
		deviceDesc.queueCreateInfoCount    = queue_create_infos.size();
		deviceDesc.pEnabledFeatures        = &deviceFeatures;
		deviceDesc.enabledExtensionCount   = enabled_device_extensions.size();
		deviceDesc.ppEnabledExtensionNames = enabled_device_extensions.data();
		deviceDesc.enabledLayerCount       = enabled_validation_layers.size();
		deviceDesc.ppEnabledLayerNames     = enabled_validation_layers.data();
		deviceDesc.pNext                   = &vulkan12features;

		const vk::Result res = m_vulkanPhysicalDevice.createDevice(&deviceDesc, nullptr, &m_vulkanDevice);
		if (res != vk::Result::eSuccess)
		{
			TST_ERROR("Failed to create a Vulkan physical device", vk::to_string(res));
			return EError::eFailedToCreate;
		}

		m_vulkanDevice.getQueue(m_queueFamilyIndices.graphics, 0, &m_graphicsQueue);
		m_vulkanDevice.getQueue(m_queueFamilyIndices.compute, 0, &m_computeQueue);
		m_vulkanDevice.getQueue(m_queueFamilyIndices.transfer, 0, &m_transferQueue);
		m_vulkanDevice.getQueue(m_queueFamilyIndices.present, 0, &m_presentQueue);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_vulkanDevice);

		m_bufferDeviceAddressSupported = vulkan12features.bufferDeviceAddress;

		TST_INFO("Created vulkan device");

		return EError::eOk;
	}

	EError VulkanDeviceManager::_pickPhysicalDevice()
	{
		auto devices = m_vulkanInstance.enumeratePhysicalDevices();

		std::vector<vk::PhysicalDevice> discreteGPUs;
		std::vector<vk::PhysicalDevice> otherGPUs;

		std::ostringstream oss;

		for (auto device: devices)
		{
			vk::PhysicalDeviceProperties prop = device.getProperties();

			oss << "\n" << prop.deviceName;

			std::unordered_set<std::string> required_device_extensions = m_enabledDeviceExtensions;

			auto device_extensions = device.enumerateDeviceExtensionProperties();

			for (const auto &ext: device_extensions)
			{
				required_device_extensions.erase(std::string(ext.extensionName.data()));
			}

			bool device_is_good = true;

			if (!required_device_extensions.empty())
			{
				for (const auto &ext: required_device_extensions)
				{
					oss << "\n\t" << ext;
				}
				device_is_good = false;
			}

			auto device_features = device.getFeatures();
			if (!device_features.samplerAnisotropy)
			{
				oss << "\n\tDevice does not support sampler anisotropy";
				device_is_good = false;
			}
			if (!device_features.textureCompressionBC)
			{
				oss << "\n\tDevice does not support texture compression block";
				device_is_good = false;
			}

			if (!_findQueueFamilies(device))
			{
				oss << "\n\tDevice does not support the required queue families";
				device_is_good = false;
			}

			if (!device_is_good)
			{
				continue;
			}

			if (prop.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				discreteGPUs.push_back(device);
			}
			else
			{
				otherGPUs.push_back(device);
			}
		}

		if (!discreteGPUs.empty())
		{
			m_vulkanPhysicalDevice = discreteGPUs[0];
			return EError::eOk;
		}

		if (!otherGPUs.empty())
		{
			m_vulkanPhysicalDevice = otherGPUs[0];
			return EError::eOk;
		}

		TST_ERROR("{}", oss.str());

		return EError::eDeviceError;
	}

	bool VulkanDeviceManager::_findQueueFamilies(vk::PhysicalDevice physical_device)
	{
		auto props = physical_device.getQueueFamilyProperties();

		for (int i = 0; i < props.size(); i++)
		{
			const auto &queueFamily = props[i];

			if (m_queueFamilyIndices.graphics == -1)
			{
				if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
				{
					m_queueFamilyIndices.graphics = i;
				}
			}

			if (m_queueFamilyIndices.compute == -1)
			{
				if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) && !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
				{
					m_queueFamilyIndices.compute = i;
				}
			}

			if (m_queueFamilyIndices.transfer == -1)
			{
				if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) && !(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) && !(
						queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
				{
					m_queueFamilyIndices.transfer = i;
				}
			}

			if (m_queueFamilyIndices.present == -1)
			{
				if (queueFamily.queueCount > 0 && glfwGetPhysicalDevicePresentationSupport(m_vulkanInstance, physical_device, i))
				{
					m_queueFamilyIndices.present = i;
				}
			}
		}

		if (m_queueFamilyIndices.graphics == -1 || m_queueFamilyIndices.present == -1 || (m_queueFamilyIndices.compute == -1) || m_queueFamilyIndices.transfer == -1)
		{
			return false;
		}

		return true;
	}
}
