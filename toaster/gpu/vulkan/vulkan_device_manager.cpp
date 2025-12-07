#include "vulkan_device_manager.hpp"

#include "error_macros.hpp"

#include <iterator>
#include <algorithm>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace tst
{
	static std::vector<const char *> stringSetToVector(const std::unordered_set<std::string> &set)
	{
		std::vector<const char *> ret;
		for (const auto &s: set)
		{
			ret.push_back(s.c_str());
		}

		return ret;
	}

	bool VulkanDeviceManager::createInstance()
	{
		if (!m_DeviceParams.headlessDevice)
		{
			if (!glfwVulkanSupported())
			{
				TST_ERROR("GLFW reports that Vulkan is not supported. Perhaps missing a call to glfwInit()?");
				return false;
			}

			// add any extensions required by GLFW
			uint32_t     glfwExtCount;
			const char **glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
			TST_ASSERT(glfwExt);

			for (uint32_t i = 0; i < glfwExtCount; i++)
			{
				enabledExtensions.instance.insert(std::string(glfwExt[i]));
			}
		}

		// add instance extensions requested by the user
		for (const std::string &name: m_DeviceParams.requiredVulkanInstanceExtensions)
		{
			enabledExtensions.instance.insert(name);
		}
		for (const std::string &name: m_DeviceParams.optionalVulkanInstanceExtensions)
		{
			optionalExtensions.instance.insert(name);
		}

		// add layers requested by the user
		for (const std::string &name: m_DeviceParams.requiredVulkanLayers)
		{
			enabledExtensions.layers.insert(name);
		}
		for (const std::string &name: m_DeviceParams.optionalVulkanLayers)
		{
			optionalExtensions.layers.insert(name);
		}

		std::unordered_set<std::string> requiredExtensions = enabledExtensions.instance;

		// figure out which optional extensions are supported
		for (const auto &instanceExt: vk::enumerateInstanceExtensionProperties())
		{
			const std::string name = instanceExt.extensionName;
			if (optionalExtensions.instance.find(name) != optionalExtensions.instance.end())
			{
				enabledExtensions.instance.insert(name);
			}

			requiredExtensions.erase(name);
		}

		if (!requiredExtensions.empty())
		{
			std::stringstream ss;
			ss << "Cannot create a Vulkan instance because the following required extension(s) are not supported:";
			for (const auto &ext: requiredExtensions)
				ss << std::endl << "  - " << ext;

			TST_ERROR("{}", ss.str().c_str());
			return false;
		}

		TST_INFO("Enabled Vulkan instance extensions:");
		for (const auto &ext: enabledExtensions.instance)
		{
			TST_INFO("    {}", ext.c_str());
		}

		std::unordered_set<std::string> requiredLayers = enabledExtensions.layers;

		for (const auto &layer: vk::enumerateInstanceLayerProperties())
		{
			const std::string name = layer.layerName;
			if (optionalExtensions.layers.find(name) != optionalExtensions.layers.end())
			{
				enabledExtensions.layers.insert(name);
			}

			requiredLayers.erase(name);
		}

		if (!requiredLayers.empty())
		{
			std::stringstream ss;
			ss << "Cannot create a Vulkan instance because the following required layer(s) are not supported:";
			for (const auto &ext: requiredLayers)
				ss << std::endl << "  - " << ext;

			TST_ERROR("{}", ss.str().c_str());
			return false;
		}

		TST_INFO("Enabled Vulkan layers:");
		for (const auto &layer: enabledExtensions.layers)
		{
			TST_INFO("    {}", layer.c_str());
		}

		auto instanceExtVec = stringSetToVector(enabledExtensions.instance);
		auto layerVec       = stringSetToVector(enabledExtensions.layers);

		auto applicationInfo = vk::ApplicationInfo();

		// Query the Vulkan API version supported on the system to make sure we use at least 1.3 when that's present.
		vk::Result res = vk::enumerateInstanceVersion(&applicationInfo.apiVersion);

		if (res != vk::Result::eSuccess)
		{
			TST_ERROR("Call to vkEnumerateInstanceVersion failed, error code = {}", nvrhi::vulkan::resultToString(VkResult(res)));
			return false;
		}

		const uint32_t minimumVulkanVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

		// Check if the Vulkan API version is sufficient.
		if (applicationInfo.apiVersion < minimumVulkanVersion)
		{
			TST_ERROR("The Vulkan API version supported on the system ({}.{}.{}) is too low, at least {}.{}.{} is required.",
					  VK_API_VERSION_MAJOR(applicationInfo.apiVersion), VK_API_VERSION_MINOR(applicationInfo.apiVersion),
					  VK_API_VERSION_PATCH(applicationInfo.apiVersion), VK_API_VERSION_MAJOR(minimumVulkanVersion), VK_API_VERSION_MINOR(minimumVulkanVersion),
					  VK_API_VERSION_PATCH(minimumVulkanVersion));
			return false;
		}

		// Spec says: A non-zero variant indicates the API is a variant of the Vulkan API and applications will typically need to be modified to run against it.
		if (VK_API_VERSION_VARIANT(applicationInfo.apiVersion) != 0)
		{
			TST_ERROR("The Vulkan API supported on the system uses an unexpected variant: {}.", VK_API_VERSION_VARIANT(applicationInfo.apiVersion));
			return false;
		}

		// Create the vulkan instance
		vk::InstanceCreateInfo info = vk::InstanceCreateInfo().setEnabledLayerCount(uint32_t(layerVec.size())).setPpEnabledLayerNames(layerVec.data()).
				setEnabledExtensionCount(uint32_t(instanceExtVec.size())).setPpEnabledExtensionNames(instanceExtVec.data()).setPApplicationInfo(&applicationInfo);

		res = vk::createInstance(&info, nullptr, &m_VulkanInstance);
		if (res != vk::Result::eSuccess)
		{
			TST_ERROR("Failed to create a Vulkan instance, error code = {}", nvrhi::vulkan::resultToString(VkResult(res)));
			return false;
		}

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_VulkanInstance);

		return true;
	}

	void VulkanDeviceManager::installDebugCallback()
	{
		auto info = vk::DebugReportCallbackCreateInfoEXT().setFlags(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning |
																	//   vk::DebugReportFlagBitsEXT::eInformation |
																	vk::DebugReportFlagBitsEXT::ePerformanceWarning).setPfnCallback(vulkanDebugCallback).
				setPUserData(this);

		vk::Result res = m_VulkanInstance.createDebugReportCallbackEXT(&info, nullptr, &m_DebugReportCallback);
		TST_ASSERT(res == vk::Result::eSuccess);
	}

	bool VulkanDeviceManager::pickPhysicalDevice()
	{
		//VkFormat requestedFormat = nvrhi::vulkan::convertFormat(m_DeviceParams.swapChainFormat);
		//vk::Extent2D requestedExtent(m_DeviceParams.backBufferWidth, m_DeviceParams.backBufferHeight);

		auto devices = m_VulkanInstance.enumeratePhysicalDevices();

		int firstDevice = 0;
		int lastDevice  = int(devices.size()) - 1;
		if (m_DeviceParams.adapterIndex >= 0)
		{
			if (m_DeviceParams.adapterIndex > lastDevice)
			{
				TST_ERROR("The specified Vulkan physical device {} does not exist.", m_DeviceParams.adapterIndex);
				return false;
			}
			firstDevice = m_DeviceParams.adapterIndex;
			lastDevice  = m_DeviceParams.adapterIndex;
		}

		// Start building an error message in case we cannot find a device.
		std::stringstream errorStream;
		errorStream << "Cannot find a Vulkan device that supports all the required extensions and properties.";

		// build a list of GPUs
		std::vector<vk::PhysicalDevice> discreteGPUs;
		std::vector<vk::PhysicalDevice> otherGPUs;
		for (int deviceIndex = firstDevice; deviceIndex <= lastDevice; ++deviceIndex)
		{
			vk::PhysicalDevice const &   dev  = devices[deviceIndex];
			vk::PhysicalDeviceProperties prop = dev.getProperties();

			errorStream << std::endl << prop.deviceName.data() << ":";

			// check that all required device extensions are present
			std::unordered_set<std::string> requiredExtensions = enabledExtensions.device;
			auto                            deviceExtensions   = dev.enumerateDeviceExtensionProperties();
			for (const auto &ext: deviceExtensions)
			{
				requiredExtensions.erase(std::string(ext.extensionName.data()));
			}

			bool deviceIsGood = true;

			if (!requiredExtensions.empty())
			{
				// device is missing one or more required extensions
				for (const auto &ext: requiredExtensions)
				{
					errorStream << std::endl << "  - missing " << ext;
				}
				deviceIsGood = false;
			}

			auto deviceFeatures = dev.getFeatures();
			if (!deviceFeatures.samplerAnisotropy)
			{
				// device is a toaster oven
				errorStream << std::endl << "  - does not support samplerAnisotropy";
				deviceIsGood = false;
			}
			if (!deviceFeatures.textureCompressionBC)
			{
				errorStream << std::endl << "  - does not support textureCompressionBC";
				deviceIsGood = false;
			}

			if (!FindQueueFamilies(dev))
			{
				// device doesn't have all the queue families we need
				errorStream << std::endl << "  - does not support the necessary queue types";
				deviceIsGood = false;
			}

			if (!deviceIsGood)
				continue;

			if (prop.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				discreteGPUs.push_back(dev);
			}
			else
			{
				otherGPUs.push_back(dev);
			}
		}

		// pick the first discrete GPU if it exists, otherwise the first integrated GPU
		if (!discreteGPUs.empty())
		{
			m_VulkanPhysicalDevice = discreteGPUs[0];
			return true;
		}

		if (!otherGPUs.empty())
		{
			m_VulkanPhysicalDevice = otherGPUs[0];
			return true;
		}

		TST_ERROR("{}", errorStream.str().c_str());

		return false;
	}

	bool VulkanDeviceManager::FindQueueFamilies(vk::PhysicalDevice physicalDevice)
	{
		auto props = physicalDevice.getQueueFamilyProperties();

		for (int i = 0; i < int(props.size()); i++)
		{
			const auto &queueFamily = props[i];

			if (m_QueueFamilyIndices.Graphics == -1)
			{
				if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
				{
					m_QueueFamilyIndices.Graphics = i;
				}
			}

			if (m_QueueFamilyIndices.Compute == -1)
			{
				if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) && !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
				{
					m_QueueFamilyIndices.Compute = i;
				}
			}

			if (m_QueueFamilyIndices.Transfer == -1)
			{
				if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) && !(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) && !(
						queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
				{
					m_QueueFamilyIndices.Transfer = i;
				}
			}

			if (m_QueueFamilyIndices.Present == -1)
			{
				if (queueFamily.queueCount > 0 && glfwGetPhysicalDevicePresentationSupport(m_VulkanInstance, physicalDevice, i))
				{
					m_QueueFamilyIndices.Present = i;
				}
			}
		}

		if (m_QueueFamilyIndices.Graphics == -1 || m_QueueFamilyIndices.Present == -1 && !m_DeviceParams.headlessDevice || (
				m_QueueFamilyIndices.Compute == -1 && m_DeviceParams.enableComputeQueue) || (m_QueueFamilyIndices.Transfer == -1 && m_DeviceParams.enableCopyQueue))
		{
			return false;
		}

		return true;
	}

	bool VulkanDeviceManager::createDevice()
	{
		// figure out which optional extensions are supported
		auto deviceExtensions = m_VulkanPhysicalDevice.enumerateDeviceExtensionProperties();
		for (const auto &ext: deviceExtensions)
		{
			const std::string name = ext.extensionName;
			if (optionalExtensions.device.find(name) != optionalExtensions.device.end())
			{
				if (name == VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME && m_DeviceParams.headlessDevice)
					continue;

				enabledExtensions.device.insert(name);
			}

			if (m_DeviceParams.enableRayTracingExtensions && m_RayTracingExtensions.find(name) != m_RayTracingExtensions.end())
			{
				enabledExtensions.device.insert(name);
			}
		}

		if (!m_DeviceParams.headlessDevice)
		{
			enabledExtensions.device.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}

		const vk::PhysicalDeviceProperties physicalDeviceProperties = m_VulkanPhysicalDevice.getProperties();
		m_RendererString                                            = std::string(physicalDeviceProperties.deviceName.data());

		bool accelStructSupported      = false;
		bool rayPipelineSupported      = false;
		bool rayQuerySupported         = false;
		bool meshletsSupported         = false;
		bool vrsSupported              = false;
		bool synchronization2Supported = false;
		bool maintenance4Supported     = false;

		TST_INFO("Enabled Vulkan device extensions:");
		for (const auto &ext: enabledExtensions.device)
		{
			TST_INFO("    {}", ext.c_str());

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
			else if (ext == VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME)
				m_SwapChainMutableFormatSupported = true;
		}

		#define APPEND_EXTENSION(condition, desc) if (condition) { (desc).pNext = pNext; pNext = &(desc); }  // NOLINT(cppcoreguidelines-macro-usage)
		void *pNext = nullptr;

		vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2;
		// Determine support for Buffer Device Address, the Vulkan 1.2 way
		auto bufferDeviceAddressFeatures = vk::PhysicalDeviceBufferDeviceAddressFeatures();
		// Determine support for maintenance4
		auto maintenance4Features = vk::PhysicalDeviceMaintenance4Features();

		// Put the user-provided extension structure at the end of the chain
		pNext = m_DeviceParams.physicalDeviceFeatures2Extensions;
		APPEND_EXTENSION(true, bufferDeviceAddressFeatures);
		APPEND_EXTENSION(maintenance4Supported, maintenance4Features);

		physicalDeviceFeatures2.pNext = pNext;
		m_VulkanPhysicalDevice.getFeatures2(&physicalDeviceFeatures2);

		std::unordered_set<int> uniqueQueueFamilies = {m_QueueFamilyIndices.Graphics};

		if (!m_DeviceParams.headlessDevice)
			uniqueQueueFamilies.insert(m_QueueFamilyIndices.Present);

		if (m_DeviceParams.enableComputeQueue)
			uniqueQueueFamilies.insert(m_QueueFamilyIndices.Compute);

		if (m_DeviceParams.enableCopyQueue)
			uniqueQueueFamilies.insert(m_QueueFamilyIndices.Transfer);

		float                                  priority = 1.f;
		std::vector<vk::DeviceQueueCreateInfo> queueDesc;
		queueDesc.reserve(uniqueQueueFamilies.size());
		for (int queueFamily: uniqueQueueFamilies)
		{
			queueDesc.push_back(vk::DeviceQueueCreateInfo().setQueueFamilyIndex(queueFamily).setQueueCount(1).setPQueuePriorities(&priority));
		}

		auto accelStructFeatures = vk::PhysicalDeviceAccelerationStructureFeaturesKHR().setAccelerationStructure(true);
		auto rayPipelineFeatures = vk::PhysicalDeviceRayTracingPipelineFeaturesKHR().setRayTracingPipeline(true).setRayTraversalPrimitiveCulling(true);
		auto rayQueryFeatures    = vk::PhysicalDeviceRayQueryFeaturesKHR().setRayQuery(true);
		auto meshletFeatures     = vk::PhysicalDeviceMeshShaderFeaturesNV().setTaskShader(true).setMeshShader(true);
		auto vrsFeatures         = vk::PhysicalDeviceFragmentShadingRateFeaturesKHR().setPipelineFragmentShadingRate(true).setPrimitiveFragmentShadingRate(true).
				setAttachmentFragmentShadingRate(true);
		auto vulkan13features = vk::PhysicalDeviceVulkan13Features().setSynchronization2(synchronization2Supported).setMaintenance4(maintenance4Features.maintenance4);

		pNext = nullptr;
		APPEND_EXTENSION(accelStructSupported, accelStructFeatures)
		APPEND_EXTENSION(rayPipelineSupported, rayPipelineFeatures)
		APPEND_EXTENSION(rayQuerySupported, rayQueryFeatures)
		APPEND_EXTENSION(meshletsSupported, meshletFeatures)
		APPEND_EXTENSION(vrsSupported, vrsFeatures)
		APPEND_EXTENSION(physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_3, vulkan13features)
		APPEND_EXTENSION(physicalDeviceProperties.apiVersion < VK_API_VERSION_1_3 && maintenance4Supported, maintenance4Features);
		#undef APPEND_EXTENSION

		auto deviceFeatures = vk::PhysicalDeviceFeatures().setShaderImageGatherExtended(true).setSamplerAnisotropy(true).setTessellationShader(true).
				setTextureCompressionBC(true).setGeometryShader(true).setImageCubeArray(true).setDualSrcBlend(true).setIndependentBlend(true).setFillModeNonSolid(true).
				setWideLines(true);

		auto vulkan12features = vk::PhysicalDeviceVulkan12Features().setDescriptorIndexing(true).setRuntimeDescriptorArray(true).setDescriptorBindingPartiallyBound(true).
				setDescriptorBindingVariableDescriptorCount(true).setTimelineSemaphore(true).setShaderSampledImageArrayNonUniformIndexing(true).
				setBufferDeviceAddress(bufferDeviceAddressFeatures.bufferDeviceAddress).setPNext(pNext);

		auto layerVec = stringSetToVector(enabledExtensions.layers);
		auto extVec   = stringSetToVector(enabledExtensions.device);

		auto deviceDesc = vk::DeviceCreateInfo().setPQueueCreateInfos(queueDesc.data()).setQueueCreateInfoCount(uint32_t(queueDesc.size())).
				setPEnabledFeatures(&deviceFeatures).setEnabledExtensionCount(uint32_t(extVec.size())).setPpEnabledExtensionNames(extVec.data()).
				setEnabledLayerCount(uint32_t(layerVec.size())).setPpEnabledLayerNames(layerVec.data()).setPNext(&vulkan12features);

		if (m_DeviceParams.deviceCreateInfoCallback)
			m_DeviceParams.deviceCreateInfoCallback(deviceDesc);

		const vk::Result res = m_VulkanPhysicalDevice.createDevice(&deviceDesc, nullptr, &m_VulkanDevice);
		if (res != vk::Result::eSuccess)
		{
			TST_ERROR("Failed to create a Vulkan physical device, error code = {}", nvrhi::vulkan::resultToString(VkResult(res)));
			return false;
		}

		m_VulkanDevice.getQueue(m_QueueFamilyIndices.Graphics, 0, &m_GraphicsQueue);
		if (m_DeviceParams.enableComputeQueue)
			m_VulkanDevice.getQueue(m_QueueFamilyIndices.Compute, 0, &m_ComputeQueue);
		if (m_DeviceParams.enableCopyQueue)
			m_VulkanDevice.getQueue(m_QueueFamilyIndices.Transfer, 0, &m_TransferQueue);
		if (!m_DeviceParams.headlessDevice)
			m_VulkanDevice.getQueue(m_QueueFamilyIndices.Present, 0, &m_PresentQueue);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_VulkanDevice);

		// remember the bufferDeviceAddress feature enablement
		m_BufferDeviceAddressSupported = vulkan12features.bufferDeviceAddress;

		TST_INFO("Created Vulkan device: {}", m_RendererString.c_str());

		return true;
	}

	#define CHECK(a) if (!(a)) { return false; }

	bool VulkanDeviceManager::CreateInstanceInternal()
	{
		if (m_DeviceParams.enableDebugRuntime)
		{
			enabledExtensions.instance.insert("VK_EXT_debug_report");
			enabledExtensions.layers.insert("VK_LAYER_KHRONOS_validation");
		}

		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = m_dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		return createInstance();
	}

	bool VulkanDeviceManager::EnumerateAdapters(std::vector<AdapterInfo> &outAdapters)
	{
		if (!m_VulkanInstance)
			return false;

		std::vector<vk::PhysicalDevice> devices = m_VulkanInstance.enumeratePhysicalDevices();
		outAdapters.clear();

		for (auto physicalDevice: devices)
		{
			vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

			AdapterInfo adapterInfo;
			adapterInfo.name                 = properties.deviceName.data();
			adapterInfo.vendorID             = properties.vendorID;
			adapterInfo.deviceID             = properties.deviceID;
			adapterInfo.vkPhysicalDevice     = physicalDevice;
			adapterInfo.dedicatedVideoMemory = 0;

			// Go through the memory types to figure out the amount of VRAM on this physical device.
			vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
			for (uint32_t heapIndex = 0; heapIndex < memoryProperties.memoryHeapCount; ++heapIndex)
			{
				vk::MemoryHeap const &heap = memoryProperties.memoryHeaps[heapIndex];
				if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
				{
					adapterInfo.dedicatedVideoMemory += heap.size;
				}
			}

			outAdapters.push_back(std::move(adapterInfo));
		}

		return true;
	}

	bool VulkanDeviceManager::CreateDevice()
	{
		if (m_DeviceParams.enableDebugRuntime)
		{
			installDebugCallback();
		}

		// add device extensions requested by the user
		for (const std::string &name: m_DeviceParams.requiredVulkanDeviceExtensions)
		{
			enabledExtensions.device.insert(name);
		}
		for (const std::string &name: m_DeviceParams.optionalVulkanDeviceExtensions)
		{
			optionalExtensions.device.insert(name);
		}

		if (!m_DeviceParams.headlessDevice)
		{
			// Need to adjust the swap chain format before creating the device because it affects physical device selection
			if (m_DeviceParams.swapChainFormat == nvrhi::Format::SRGBA8_UNORM)
				m_DeviceParams.swapChainFormat = nvrhi::Format::SBGRA8_UNORM;
			else if (m_DeviceParams.swapChainFormat == nvrhi::Format::RGBA8_UNORM)
				m_DeviceParams.swapChainFormat = nvrhi::Format::BGRA8_UNORM;
		}
		CHECK(pickPhysicalDevice())
		CHECK(FindQueueFamilies(m_VulkanPhysicalDevice))
		CHECK(createDevice())

		auto vecInstanceExt = stringSetToVector(enabledExtensions.instance);
		auto vecLayers      = stringSetToVector(enabledExtensions.layers);
		auto vecDeviceExt   = stringSetToVector(enabledExtensions.device);

		nvrhi::vulkan::DeviceDesc deviceDesc;
		deviceDesc.errorCB            = &DefaultMessageCallback::GetInstance();
		deviceDesc.instance           = m_VulkanInstance;
		deviceDesc.physicalDevice     = m_VulkanPhysicalDevice;
		deviceDesc.device             = m_VulkanDevice;
		deviceDesc.graphicsQueue      = m_GraphicsQueue;
		deviceDesc.graphicsQueueIndex = m_QueueFamilyIndices.Graphics;
		if (m_DeviceParams.enableComputeQueue)
		{
			deviceDesc.computeQueue      = m_ComputeQueue;
			deviceDesc.computeQueueIndex = m_QueueFamilyIndices.Compute;
		}
		if (m_DeviceParams.enableCopyQueue)
		{
			deviceDesc.transferQueue      = m_TransferQueue;
			deviceDesc.transferQueueIndex = m_QueueFamilyIndices.Transfer;
		}
		deviceDesc.instanceExtensions           = vecInstanceExt.data();
		deviceDesc.numInstanceExtensions        = vecInstanceExt.size();
		deviceDesc.deviceExtensions             = vecDeviceExt.data();
		deviceDesc.numDeviceExtensions          = vecDeviceExt.size();
		deviceDesc.bufferDeviceAddressSupported = m_BufferDeviceAddressSupported;

		m_NvrhiDevice = nvrhi::vulkan::createDevice(deviceDesc);

		if (m_DeviceParams.enableNvrhiValidationLayer)
		{
			m_ValidationLayer = nvrhi::validation::createValidationLayer(m_NvrhiDevice);
		}

		return true;
	}

	bool VulkanDeviceManager::InitSurfaceCapabilities(uint64_t surfaceHandle)
	{
		#if TODO
		vk::SurfaceKHR windowSurface = (VkSurfaceKHR) surfaceHandle; if (windowSurface)
		{
			// check that this device supports our intended swap chain creation parameters
			auto surfaceCaps   = dev.getSurfaceCapabilitiesKHR(windowSurface);
			auto surfaceFmts   = dev.getSurfaceFormatsKHR(windowSurface);
			auto surfacePModes = dev.getSurfacePresentModesKHR(windowSurface);

			if (surfaceCaps.minImageCount > m_DeviceParams.swapChainBufferCount || (
					surfaceCaps.maxImageCount < m_DeviceParams.swapChainBufferCount && surfaceCaps.maxImageCount > 0))
			{
				errorStream << std::endl << "  - cannot support the requested swap chain image count:";
				errorStream << " requested " << m_DeviceParams.swapChainBufferCount << ", available " << surfaceCaps.minImageCount << " - " << surfaceCaps.maxImageCount;
				deviceIsGood = false;
			}

			if (surfaceCaps.minImageExtent.width > requestedExtent.width || surfaceCaps.minImageExtent.height > requestedExtent.height || surfaceCaps.maxImageExtent.width
				< requestedExtent.width || surfaceCaps.maxImageExtent.height < requestedExtent.height)
			{
				errorStream << std::endl << "  - cannot support the requested swap chain size:";
				errorStream << " requested " << requestedExtent.width << "x" << requestedExtent.height << ", ";
				errorStream << " available " << surfaceCaps.minImageExtent.width << "x" << surfaceCaps.minImageExtent.height;
				errorStream << " - " << surfaceCaps.maxImageExtent.width << "x" << surfaceCaps.maxImageExtent.height;
				deviceIsGood = false;
			}

			bool surfaceFormatPresent = false;
			for (const vk::SurfaceFormatKHR &surfaceFmt: surfaceFmts)
			{
				if (surfaceFmt.format == vk::Format(requestedFormat))
				{
					surfaceFormatPresent = true;
					break;
				}
			}

			if (!surfaceFormatPresent)
			{
				// can't create a swap chain using the format requested
				errorStream << std::endl << "  - does not support the requested swap chain format";
				deviceIsGood = false;
			}

			// check that we can present from the graphics queue
			uint32_t canPresent = dev.getSurfaceSupportKHR(m_QueueFamilyIndices.Graphics, windowSurface);
			if (!canPresent)
			{
				errorStream << std::endl << "  - cannot present";
				deviceIsGood = false;
			}
		}
		#endif
		return true;
	}

	#undef CHECK

	void VulkanDeviceManager::DestroyDevice()
	{
		m_NvrhiDevice     = nullptr;
		m_ValidationLayer = nullptr;
		m_RendererString.clear();

		if (m_VulkanDevice)
		{
			m_VulkanDevice.destroy();
			m_VulkanDevice = nullptr;
		}

		if (m_DebugReportCallback)
		{
			m_VulkanInstance.destroyDebugReportCallbackEXT(m_DebugReportCallback);
		}

		if (m_VulkanInstance)
		{
			m_VulkanInstance.destroy();
			m_VulkanInstance = nullptr;
		}
	}

	DeviceManager *DeviceManager::CreateVK(GLFWwindow *windowHandle)
	{
		return new VulkanDeviceManager(windowHandle);
	}
}
