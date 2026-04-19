#include "vk_gpu_context.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"
#include "toast_lib/util_defines.hpp"

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
			_createCommandPools();
		}
		catch (const vk::Error &p_err)
		{
			LOG_FATAL("Vulkan error: {}", p_err.what());
		}
	}

	VKGPUContext::~VKGPUContext() noexcept = default;

	auto VKGPUContext::setCurrentFrameIndex(uint32 p_index) -> void
	{
		m_currentFrameIndex = p_index;
	}

	auto VKGPUContext::performGarbageCollection() -> void
	{
		while (!m_pendingDeletions[m_currentFrameIndex].empty())
		{
			auto deleter{std::move(m_pendingDeletions[m_currentFrameIndex].front())};
			m_pendingDeletions[m_currentFrameIndex].pop_front();
			deleter();
		}
	}

	auto VKGPUContext::getVulkanInstance() -> vk::raii::Instance &
	{
		return m_vulkanInstance;
	}

	auto VKGPUContext::getPhysicalDevice() -> vk::raii::PhysicalDevice &
	{
		return m_currentPhysicalDevice;
	}

	auto VKGPUContext::getDevice() -> vk::raii::Device &
	{
		return m_device;
	}

	auto VKGPUContext::getGraphicsQueue() -> vk::raii::Queue &
	{
		return m_graphicsQueue;
	}

	auto VKGPUContext::getTransferQueue() -> vk::raii::Queue &
	{
		return m_transferQueue;
	}

	auto VKGPUContext::getComputeQueue() -> vk::raii::Queue &
	{
		return m_computeQueue;
	}

	auto VKGPUContext::getQueueFamilyIndices() const -> const QueueFamilyIndices &
	{
		return m_queueFamilyIndices;
	}

	auto VKGPUContext::getSurface() -> vk::raii::SurfaceKHR &
	{
		return m_surface;
	}

	auto VKGPUContext::getGraphicsCommandPool() -> vk::raii::CommandPool &
	{
		return m_graphicsCommandPool;
	}

	auto VKGPUContext::getTransferCommandPool() -> vk::raii::CommandPool &
	{
		return m_transferCommandPool;
	}

	auto VKGPUContext::getComputeCommandPool() -> vk::raii::CommandPool &
	{
		return m_computeCommandPool;
	}

	auto VKGPUContext::_createInstance() -> void
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

		vk::InstanceCreateInfo instance_create_info{};
		instance_create_info.pApplicationInfo        = &app_info;
		instance_create_info.enabledExtensionCount   = required_extensions.size();
		instance_create_info.ppEnabledExtensionNames = required_extensions.data();
		// instance_create_info.flags                   = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

		if (c_enableValidationLayers)
		{
			instance_create_info.enabledLayerCount   = required_validation_layers.size();
			instance_create_info.ppEnabledLayerNames = required_validation_layers.data();
			instance_create_info.pNext               = &debug_messenger_create_info;
		}

		m_vulkanInstance = vk::raii::Instance{m_context, instance_create_info};
	}

	auto VKGPUContext::_createDebugMessenger() -> void
	{
		if (!c_enableValidationLayers)
			return;
		constexpr vk::DebugUtilsMessageSeverityFlagsEXT severity_flags{
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
		};
		constexpr vk::DebugUtilsMessageTypeFlagsEXT message_type_flags{
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
		};
		vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
		debug_messenger_create_info.messageSeverity = severity_flags;
		debug_messenger_create_info.messageType     = message_type_flags;
		debug_messenger_create_info.pfnUserCallback = &_debugCallback;

		m_debugUtilsMessenger = m_vulkanInstance.createDebugUtilsMessengerEXT(debug_messenger_create_info);
	}

	auto VKGPUContext::_createSurface() -> void
	{
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(*m_vulkanInstance, m_window, nullptr, &surface) != VK_SUCCESS)
		{
			LOG_ERROR("Failed to create window surface");
			system("pause");
			TST_ASSERT(false);
		}
		m_surface = {m_vulkanInstance, surface};
	}

	auto VKGPUContext::_pickPhysicalDevice() -> void
	{
		m_requiredDeviceExtensions = {
			vk::KHRSwapchainExtensionName,
			vk::KHRDynamicRenderingExtensionName,
			vk::KHRTimelineSemaphoreExtensionName,
			vk::EXTCustomBorderColorExtensionName,
			// vk::KHRMaintenance6ExtensionName,
			vk::KHRLoadStoreOpNoneExtensionName
		};

		auto physical_devices = m_vulkanInstance.enumeratePhysicalDevices();
		if (physical_devices.empty())
		{
			// If your gpu does not have Vulkan support, we can't use Vulkan
			LOG_ERROR("Failed to find physical devices with Vulkan support");
			system("pause");
			TST_ASSERT(false);
		}

		const auto device_it = std::ranges::find_if(physical_devices, [this](const auto &device)
		{
			return _isDeviceSuitable(device);
		});
		if (device_it == physical_devices.end())
		{
			LOG_ERROR("Failed to find suitable physical device");
			system("pause");
			TST_ASSERT(false);
		}

		m_currentPhysicalDevice = *device_it;

		vk::PhysicalDeviceProperties props{m_currentPhysicalDevice.getProperties()};

		vk::SampleCountFlags sample_counts{props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts};
		if (sample_counts & vk::SampleCountFlagBits::e64)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e64;
		else if (sample_counts & vk::SampleCountFlagBits::e32)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e32;
		else if (sample_counts & vk::SampleCountFlagBits::e16)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e16;
		else if (sample_counts & vk::SampleCountFlagBits::e8)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e8;
		else if (sample_counts & vk::SampleCountFlagBits::e4)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e4;
		else if (sample_counts & vk::SampleCountFlagBits::e2)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e2;
		else
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e1;

		m_depthFormat = findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal,
											vk::FormatFeatureFlagBits::eDepthStencilAttachment);

		LOG_INFO("Using physical device: {} | Device ID: {}\n", props.deviceName.data(), props.deviceID);

		LOG_INFO("Available device extensions:");
		auto extension_props = m_currentPhysicalDevice.enumerateDeviceExtensionProperties();
		for (auto ext: extension_props)
			LOG_INFO("\t{}", ext.extensionName.data());
		LOG_INFO("");
	}

	auto VKGPUContext::_createLogicalDevice() -> void
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
		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			if (queue_family_props[i].queueFlags & vk::QueueFlagBits::eCompute && queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				m_queueFamilyIndices.compute = i;
				break;
			}
		}
		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			if (queue_family_props[i].queueFlags & vk::QueueFlagBits::eTransfer && (
					static_cast<uint32>(queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics) == 0) && (
					static_cast<uint32>(queue_family_props[i].queueFlags & vk::QueueFlagBits::eCompute) == 0))
			{
				m_queueFamilyIndices.transfer = i;
				break;
			}
		}

		if (m_queueFamilyIndices.graphics == UINT32_MAX)
		{
			LOG_ERROR("Failed to find a queue family that supports present");
			TST_ASSERT(false);
		}

		if (m_queueFamilyIndices.transfer == UINT32_MAX)
		{
			LOG_ERROR("Failed to find a transfer queue family");
			TST_ASSERT(false);
		}

		if (m_queueFamilyIndices.compute == UINT32_MAX)
		{
			LOG_ERROR("Failed to find a compute queue family");
			TST_ASSERT(false);
		}

		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceCustomBorderColorFeaturesEXT,
			vk::PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT> feature_chain{{}, {}, {}, {}, {}, {}};
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy                                           = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading                                           = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.fillModeNonSolid                                            = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore                                             = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering                                              = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().synchronization2                                              = true;
		feature_chain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState                           = true;
		feature_chain.get<vk::PhysicalDeviceCustomBorderColorFeaturesEXT>().customBorderColors                                = true;
		feature_chain.get<vk::PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT>().dynamicRenderingUnusedAttachments = true;

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};

		bool has_separate_compute_queue = m_queueFamilyIndices.graphics != m_queueFamilyIndices.compute;

		// Create the graphics and present queue
		// The graphics queue should be the same as the present one
		auto &graphics_queue_create_info            = queue_create_infos.emplace_back();
		graphics_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.graphics;
		graphics_queue_create_info.queueCount       = has_separate_compute_queue ? 1 : 2;
		std::vector<float32> queue_priorities;
		queue_priorities.emplace_back(1.0f);
		if (!has_separate_compute_queue)
			queue_priorities.emplace_back(1.0f);
		graphics_queue_create_info.pQueuePriorities = queue_priorities.data();

		constexpr float32 queue_priority = 1.0f;

		// Create the transfer queue
		auto &transfer_queue_create_info            = queue_create_infos.emplace_back();
		transfer_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.transfer;
		transfer_queue_create_info.queueCount       = 1;
		transfer_queue_create_info.pQueuePriorities = &queue_priority;

		if (has_separate_compute_queue)
		{
			// Create the compute queue
			auto &compute_queue_create_info            = queue_create_infos.emplace_back();
			compute_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.compute;
			compute_queue_create_info.queueCount       = 1;
			compute_queue_create_info.pQueuePriorities = &queue_priority;
		}

		vk::DeviceCreateInfo device_create_info{};
		device_create_info.enabledExtensionCount   = static_cast<uint32>(m_requiredDeviceExtensions.size());
		device_create_info.ppEnabledExtensionNames = m_requiredDeviceExtensions.data();
		device_create_info.queueCreateInfoCount    = queue_create_infos.size();
		device_create_info.pQueueCreateInfos       = queue_create_infos.data();
		device_create_info.pNext                   = &feature_chain.get<vk::PhysicalDeviceFeatures2>();

		m_device = {m_currentPhysicalDevice, device_create_info};

		// Create the queues
		m_graphicsQueue = {m_device, m_queueFamilyIndices.graphics, 0};
		m_transferQueue = {m_device, m_queueFamilyIndices.transfer, 0};
		if (m_queueFamilyIndices.graphics == m_queueFamilyIndices.compute)
			m_computeQueue = {m_device, m_queueFamilyIndices.compute, 1};

		LOG_TRACE("Graphics queue family index {}", m_queueFamilyIndices.graphics);
		LOG_TRACE("Transfer queue family index {}", m_queueFamilyIndices.transfer);
		LOG_TRACE("Compute queue family index {}", m_queueFamilyIndices.compute);
	}

	auto VKGPUContext::_createCommandPools() -> void
	{
		// Graphics
		vk::CommandPoolCreateInfo graphics_command_pool_create_info{};
		graphics_command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.graphics;
		graphics_command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_graphicsCommandPool = {m_device, graphics_command_pool_create_info};

		// Transfer
		vk::CommandPoolCreateInfo transfer_command_pool_create_info{};
		transfer_command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.transfer;
		transfer_command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_transferCommandPool = {m_device, transfer_command_pool_create_info};

		// Compute
		vk::CommandPoolCreateInfo compute_command_pool_create_info{};
		compute_command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.compute;
		compute_command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_computeCommandPool = {m_device, compute_command_pool_create_info};
	}

	auto VKGPUContext::_isDeviceSuitable(const vk::raii::PhysicalDevice &p_physical_device) const -> bool
	{
		auto props              = p_physical_device.getProperties();
		bool vulkan_1_3_support = props.apiVersion >= vk::ApiVersion13;

		auto queue_families    = p_physical_device.getQueueFamilyProperties();
		bool supports_graphics = std::ranges::any_of(queue_families, [](const auto &queue_family)
		{
			return !!(queue_family.queueFlags & vk::QueueFlagBits::eGraphics);
		});

		bool supports_compute = std::ranges::any_of(queue_families, [](const auto &queue_family)
		{
			return !!(queue_family.queueFlags & vk::QueueFlagBits::eCompute);
		});

		// For the moment, the only required extension is the swapchain one.
		// std::vector required_device_extensions{vk::KHRSwapchainExtensionName, vk::KHRDynamicRenderingExtensionName, vk::KHRTimelineSemaphoreExtensionName};

		// Checks if all the required extensions are present in the available_device_extensions vector.
		auto available_device_extensions             = p_physical_device.enumerateDeviceExtensionProperties();
		bool supports_all_required_device_extensions = std::ranges::all_of(m_requiredDeviceExtensions, [available_device_extensions](const auto &required_ext)
		{
			return std::ranges::any_of(available_device_extensions, [&required_ext](const auto &available_ext)
			{
				return std::strcmp(available_ext.extensionName, required_ext) == 0;
			});
		});

		for (const auto &ext: available_device_extensions)
		{
			LOG_TRACE("{}", ext.extensionName.data());
		}

		auto features = p_physical_device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceCustomBorderColorFeaturesEXT>();

		bool supports_required_features = features.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy && features.get<vk::PhysicalDeviceFeatures2>().features.
										  sampleRateShading && features.get<vk::PhysicalDeviceFeatures2>().features.fillModeNonSolid && features.get<
											  vk::PhysicalDeviceVulkan12Features>().timelineSemaphore && features.get<vk::PhysicalDeviceVulkan13Features>().
										  dynamicRendering && features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 && features.get<
											  vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState && features.get<
											  vk::PhysicalDeviceCustomBorderColorFeaturesEXT>().customBorderColors;

		return vulkan_1_3_support && supports_graphics && supports_compute && supports_all_required_device_extensions && supports_required_features;
	}

	auto VKGPUContext::_getRequiredInstanceExtensions() const -> std::vector<CString>
	{
		// Gets all the possible platform-specific extension names that glfw needs to create a window.
		// For windows, one of them will be VK_KHR_win32_surface extension
		uint32     glfw_extension_count{0u};
		const auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		// Inserts the
		std::vector<CString> required_extensions{glfw_extension_count};
		for (uint32 i{0u}; i < glfw_extension_count; ++i)
			required_extensions[i] = glfw_extensions[i];

		// required_extensions.emplace_back(vk::KHRPortabilityEnumerationExtensionName);

		if (c_enableValidationLayers)
			required_extensions.emplace_back(vk::EXTDebugUtilsExtensionName);

		return required_extensions;
	}

	auto VKGPUContext::_debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      p_message_severity, vk::DebugUtilsMessageTypeFlagsEXT p_message_type,
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

	auto VKGPUContext::transitionImageLayout(vk::raii::CommandBuffer &p_command_buffer, vk::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
											 vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
											 vk::PipelineStageFlags2  p_dst_stage_mask, vk::ImageAspectFlags p_aspect_flags) -> void
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
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, 1, 0, 1};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		p_command_buffer.pipelineBarrier2(dependency_info);
	}

	auto VKGPUContext::transitionImageLayout(vk::raii::CommandBuffer &p_command_buffer, vk::raii::Image &       p_image, vk::ImageLayout            p_old_layout,
											 vk::ImageLayout          p_new_layout, vk::AccessFlags2            p_src_access_mask, vk::AccessFlags2 p_dst_access_mask,
											 vk::PipelineStageFlags2  p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
											 vk::ImageAspectFlags     p_aspect_flags) -> void
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
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, 1, 0, 1};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		p_command_buffer.pipelineBarrier2(dependency_info);
	}

	auto VKGPUContext::createShaderModule(const std::vector<uint8> &p_code) -> vk::raii::ShaderModule
	{
		vk::ShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.codeSize = p_code.size();
		shader_module_create_info.pCode    = reinterpret_cast<const uint32 *>(p_code.data());

		return {m_device, shader_module_create_info};
	}

	auto VKGPUContext::createShaderModule(const std::vector<uint32> &p_code) -> vk::raii::ShaderModule
	{
		vk::ShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.codeSize = p_code.size() * sizeof(uint32);
		shader_module_create_info.pCode    = p_code.data();

		return {m_device, shader_module_create_info};
	}

	auto VKGPUContext::findMemoryType(uint32 p_type_filter, vk::MemoryPropertyFlags p_properties) const -> uint32
	{
		vk::PhysicalDeviceMemoryProperties memory_properties = m_currentPhysicalDevice.getMemoryProperties();
		for (uint32 i{0u}; i < memory_properties.memoryTypeCount; i++)
		{
			if ((p_type_filter & BIT(i)) && (memory_properties.memoryTypes[i].propertyFlags & p_properties) == p_properties)
			{
				return i;
			}
		}
		TST_ASSERT_MSG(false, "Failed to find a matching memory type!");
		return UINT32_MAX;
	}

	auto VKGPUContext::createBuffer(vk::DeviceSize    p_size, vk::BufferUsageFlags          p_usage_flags, vk::MemoryPropertyFlags p_memory_properties,
									vk::raii::Buffer &p_out_buffer, vk::raii::DeviceMemory &p_out_memory) const -> void
	{
		vk::BufferCreateInfo buffer_create_info{};
		buffer_create_info.size        = p_size;
		buffer_create_info.usage       = p_usage_flags;
		buffer_create_info.sharingMode = vk::SharingMode::eConcurrent;

		buffer_create_info.queueFamilyIndexCount = 2;
		uint32 qfi[]                             = {m_queueFamilyIndices.graphics, m_queueFamilyIndices.transfer};
		buffer_create_info.pQueueFamilyIndices   = qfi;

		p_out_buffer = {m_device, buffer_create_info};

		vk::MemoryRequirements memory_requirements = p_out_buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, p_memory_properties);
		memory_allocate_info.allocationSize  = memory_requirements.size;

		p_out_memory = {m_device, memory_allocate_info};

		p_out_buffer.bindMemory(p_out_memory, 0u);
	}

	auto VKGPUContext::copyBuffer(vk::raii::Buffer &p_src_buffer, vk::raii::Buffer &p_dst_buffer, vk::DeviceSize p_size) const -> void
	{
		vk::BufferCopy2 buffer_copy{};
		buffer_copy.size      = static_cast<uint32>(p_size);
		buffer_copy.srcOffset = 0;
		buffer_copy.dstOffset = 0;

		vk::CopyBufferInfo2 copy_info{};
		copy_info.srcBuffer   = p_src_buffer;
		copy_info.dstBuffer   = p_dst_buffer;
		copy_info.regionCount = 1;
		copy_info.pRegions    = &buffer_copy;

		vk::raii::CommandBuffer cmd = beginSingleTimeCommandsTransfer();
		cmd.copyBuffer2(copy_info);
		endSingleTimeCommandsTransfer(cmd);
	}

	auto VKGPUContext::createImage(uint32           p_width, uint32 p_height, uint32 p_mip_levels, vk::SampleCountFlagBits p_sample_count, vk::Format p_format,
								   vk::ImageTiling  p_image_tiling, vk::ImageUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties,
								   vk::raii::Image &p_out_image, vk::raii::DeviceMemory &p_out_memory) const -> void
	{
		vk::ImageCreateInfo image_create_info{};
		image_create_info.extent.width  = p_width;
		image_create_info.extent.height = p_height;
		image_create_info.extent.depth  = 1;
		image_create_info.mipLevels     = p_mip_levels;
		image_create_info.arrayLayers   = 1;
		image_create_info.imageType     = vk::ImageType::e2D;
		image_create_info.samples       = p_sample_count;
		image_create_info.sharingMode   = vk::SharingMode::eConcurrent;
		image_create_info.tiling        = p_image_tiling;
		image_create_info.initialLayout = vk::ImageLayout::eUndefined;
		image_create_info.usage         = p_usage_flags;
		image_create_info.format        = p_format;

		image_create_info.queueFamilyIndexCount = 2;
		uint32 qfi[]                            = {m_queueFamilyIndices.graphics, m_queueFamilyIndices.transfer};
		image_create_info.pQueueFamilyIndices   = qfi;

		p_out_image                                = {m_device, image_create_info};
		vk::MemoryRequirements memory_requirements = p_out_image.getMemoryRequirements();
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.allocationSize  = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, p_memory_properties);

		p_out_memory = {m_device, memory_allocate_info};

		p_out_image.bindMemory(p_out_memory, 0u);
	}

	auto VKGPUContext::transitionImageLayout(vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
											 vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
											 uint32           p_mip_levels, vk::ImageAspectFlags p_aspect_flags) const -> void
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
		image_memory_barrier.subresourceRange    = {p_aspect_flags, 0, p_mip_levels, 0, 1};

		vk::DependencyInfo dependency_info{};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers    = &image_memory_barrier;

		vk::raii::CommandBuffer cmd = beginSingleTimeCommandsGraphics();
		cmd.pipelineBarrier2(dependency_info);
		endSingleTimeCommandsGraphics(cmd);
	}

	auto VKGPUContext::copyBufferToImage(vk::raii::Buffer &p_src_buffer, vk::raii::Image &p_dst_image, uint32 p_width, uint32 p_height) const -> void
	{
		vk::BufferImageCopy2 image_copy{};
		image_copy.bufferOffset      = 0;
		image_copy.bufferRowLength   = 0;
		image_copy.bufferImageHeight = 0;
		image_copy.imageOffset       = vk::Offset3D{0, 0, 0};
		image_copy.imageExtent       = vk::Extent3D{p_width, p_height, 1};
		image_copy.imageSubresource  = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};

		vk::CopyBufferToImageInfo2 buffer_image_copy{};
		buffer_image_copy.srcBuffer      = p_src_buffer;
		buffer_image_copy.dstImage       = p_dst_image;
		buffer_image_copy.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
		buffer_image_copy.regionCount    = 1;
		buffer_image_copy.pRegions       = &image_copy;

		vk::raii::CommandBuffer cmd = beginSingleTimeCommandsTransfer();
		cmd.copyBufferToImage2(buffer_image_copy);
		endSingleTimeCommandsTransfer(cmd);
	}

	auto VKGPUContext::createImageView(vk::raii::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags,
									   uint32           p_mip_levels) const -> vk::raii::ImageView
	{
		vk::ImageViewCreateInfo image_view_create_info{};
		image_view_create_info.viewType   = vk::ImageViewType::e2D;
		image_view_create_info.image      = p_src_image;
		image_view_create_info.components = {
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		};
		image_view_create_info.subresourceRange = vk::ImageSubresourceRange{p_aspect_flags, 0, p_mip_levels, 0, 1};
		image_view_create_info.format           = p_format;

		return {m_device, image_view_create_info};
	}

	auto VKGPUContext::createImageView(vk::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags, uint32 p_mip_levels) const -> vk::raii::ImageView
	{
		vk::ImageViewCreateInfo image_view_create_info{};
		image_view_create_info.viewType   = vk::ImageViewType::e2D;
		image_view_create_info.image      = p_src_image;
		image_view_create_info.components = {
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		};
		image_view_create_info.subresourceRange = vk::ImageSubresourceRange{p_aspect_flags, 0, p_mip_levels, 0, 1};
		image_view_create_info.format           = p_format;

		return {m_device, image_view_create_info};
	}

	auto VKGPUContext::generateMipmaps(vk::raii::Image &p_src_image, vk::Format p_format, uint32 p_width, uint32 p_height, uint32 p_mip_levels) const -> void
	{
		vk::ImageMemoryBarrier2 memory_barrier{};
		memory_barrier.image                           = p_src_image;
		memory_barrier.oldLayout                       = vk::ImageLayout::eTransferDstOptimal;
		memory_barrier.newLayout                       = vk::ImageLayout::eTransferSrcOptimal;
		memory_barrier.srcAccessMask                   = vk::AccessFlagBits2::eTransferWrite;
		memory_barrier.dstAccessMask                   = vk::AccessFlagBits2::eTransferRead;
		memory_barrier.srcQueueFamilyIndex             = vk::QueueFamilyIgnored;
		memory_barrier.dstQueueFamilyIndex             = vk::QueueFamilyIgnored;
		memory_barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
		memory_barrier.subresourceRange.baseArrayLayer = 0;
		memory_barrier.subresourceRange.baseMipLevel   = 0;
		memory_barrier.subresourceRange.layerCount     = 1;
		memory_barrier.subresourceRange.levelCount     = 1;

		int32 mip_width{static_cast<int32>(p_width)};
		int32 mip_height{static_cast<int32>(p_height)};

		vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.commandPool        = m_graphicsCommandPool;
		command_buffer_allocate_info.level              = vk::CommandBufferLevel::ePrimary;

		vk::raii::CommandBuffer command_buffer{std::move(m_device.allocateCommandBuffers(command_buffer_allocate_info).front())};

		vk::CommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		command_buffer.begin(command_buffer_begin_info);

		for (uint32 i{1u}; i < p_mip_levels; ++i)
		{
			memory_barrier.subresourceRange.baseMipLevel = i - 1;
			memory_barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
			memory_barrier.newLayout                     = vk::ImageLayout::eTransferSrcOptimal;
			memory_barrier.srcAccessMask                 = vk::AccessFlagBits2::eTransferWrite;
			memory_barrier.dstAccessMask                 = vk::AccessFlagBits2::eTransferRead;

			{
				memory_barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
				memory_barrier.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;

				vk::DependencyInfo dependency_info{};
				dependency_info.imageMemoryBarrierCount = 1;
				dependency_info.pImageMemoryBarriers    = &memory_barrier;
				command_buffer.pipelineBarrier2(dependency_info);
			}

			std::array<vk::Offset3D, 2> src_offsets;
			std::array<vk::Offset3D, 2> dst_offsets;

			src_offsets[0] = vk::Offset3D{0, 0, 0};
			src_offsets[1] = vk::Offset3D{mip_width, mip_height, 1};

			dst_offsets[0] = vk::Offset3D{0, 0, 0};
			dst_offsets[1] = vk::Offset3D{mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1};

			vk::ImageBlit2 image_blit{};
			image_blit.srcOffsets     = src_offsets;
			image_blit.dstOffsets     = dst_offsets;
			image_blit.srcSubresource = vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i - 1, 0, 1};
			image_blit.dstSubresource = vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i, 0, 1};

			vk::BlitImageInfo2 blit_info{};
			blit_info.srcImage       = p_src_image;
			blit_info.dstImage       = p_src_image;
			blit_info.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
			blit_info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
			blit_info.regionCount    = 1;
			blit_info.pRegions       = &image_blit;
			blit_info.filter         = vk::Filter::eLinear;
			command_buffer.blitImage2(blit_info);

			memory_barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
			memory_barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
			memory_barrier.srcAccessMask = vk::AccessFlagBits2::eTransferRead;
			memory_barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

			{
				memory_barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
				memory_barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;

				vk::DependencyInfo dependency_info{};
				dependency_info.imageMemoryBarrierCount = 1;
				dependency_info.pImageMemoryBarriers    = &memory_barrier;
				command_buffer.pipelineBarrier2(dependency_info);
			}

			if (mip_width > 1)
				mip_width /= 2;
			if (mip_height > 1)
				mip_height /= 2;
		}

		memory_barrier.subresourceRange.baseMipLevel = p_mip_levels - 1;
		memory_barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
		memory_barrier.newLayout                     = vk::ImageLayout::eShaderReadOnlyOptimal;
		memory_barrier.srcAccessMask                 = vk::AccessFlagBits2::eTransferWrite;
		memory_barrier.dstAccessMask                 = vk::AccessFlagBits2::eShaderRead;

		{
			memory_barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
			memory_barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;

			vk::DependencyInfo dependency_info{};
			dependency_info.imageMemoryBarrierCount = 1;
			dependency_info.pImageMemoryBarriers    = &memory_barrier;
			command_buffer.pipelineBarrier2(dependency_info);
		}

		command_buffer.end();

		vk::FenceCreateInfo fence_create_info{};
		vk::raii::Fence     wait_fence{m_device, fence_create_info};

		vk::CommandBufferSubmitInfo command_buffer_submit_info{};
		command_buffer_submit_info.commandBuffer = *command_buffer;

		vk::SubmitInfo2 submit_info{};
		submit_info.commandBufferInfoCount = 1;
		submit_info.pCommandBufferInfos    = &command_buffer_submit_info;
		m_graphicsQueue.submit2(submit_info, *wait_fence);

		vk::Result res = m_device.waitForFences(*wait_fence, true, UINT64_MAX);
		if (res != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to wait for Fence");
	}

	auto VKGPUContext::beginSingleTimeCommandsTransfer() const -> vk::raii::CommandBuffer
	{
		vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.commandPool        = m_transferCommandPool;
		command_buffer_allocate_info.level              = vk::CommandBufferLevel::ePrimary;

		vk::raii::CommandBuffer command_buffer{std::move(m_device.allocateCommandBuffers(command_buffer_allocate_info).front())};

		vk::CommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		command_buffer.begin(command_buffer_begin_info);
		return command_buffer;
	}

	auto VKGPUContext::endSingleTimeCommandsTransfer(vk::raii::CommandBuffer &p_command_buffer) const -> void
	{
		p_command_buffer.end();

		vk::FenceCreateInfo fence_create_info{};
		vk::raii::Fence     wait_fence{m_device, fence_create_info};

		vk::CommandBufferSubmitInfo command_buffer_submit_info{};
		command_buffer_submit_info.commandBuffer = *p_command_buffer;

		vk::SubmitInfo2 submit_info{};
		submit_info.commandBufferInfoCount = 1;
		submit_info.pCommandBufferInfos    = &command_buffer_submit_info;
		m_transferQueue.submit2(submit_info, *wait_fence);

		vk::Result res = m_device.waitForFences(*wait_fence, true, UINT64_MAX);
		if (res != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to wait for Fence");
	}

	auto VKGPUContext::beginSingleTimeCommandsGraphics() const -> vk::raii::CommandBuffer
	{
		vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.commandPool        = m_graphicsCommandPool;
		command_buffer_allocate_info.level              = vk::CommandBufferLevel::ePrimary;

		vk::raii::CommandBuffer command_buffer{std::move(m_device.allocateCommandBuffers(command_buffer_allocate_info).front())};

		vk::CommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		command_buffer.begin(command_buffer_begin_info);
		return command_buffer;
	}

	auto VKGPUContext::endSingleTimeCommandsGraphics(vk::raii::CommandBuffer &p_command_buffer) const -> void
	{
		p_command_buffer.end();

		vk::FenceCreateInfo fence_create_info{};
		vk::raii::Fence     wait_fence{m_device, fence_create_info};

		vk::CommandBufferSubmitInfo command_buffer_submit_info{};
		command_buffer_submit_info.commandBuffer = *p_command_buffer;

		vk::SubmitInfo2 submit_info{};
		submit_info.commandBufferInfoCount = 1;
		submit_info.pCommandBufferInfos    = &command_buffer_submit_info;
		m_graphicsQueue.submit2(submit_info, *wait_fence);

		vk::Result res = m_device.waitForFences(*wait_fence, true, UINT64_MAX);
		if (res != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to wait for Fence");
	}

	auto VKGPUContext::findSupportedFormat(const std::vector<vk::Format> &p_supported_formats, const vk::ImageTiling p_tiling,
										   const vk::FormatFeatureFlags   p_feature_flags) const -> vk::Format
	{
		for (const auto &format: p_supported_formats)
		{
			vk::FormatProperties props = m_currentPhysicalDevice.getFormatProperties(format);

			if (p_tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & p_feature_flags) == p_feature_flags)
				return format;
			if (p_tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & p_feature_flags) == p_feature_flags)
				return format;
		}
		TST_ASSERT_MSG(false, "Unsupported format");
		return vk::Format::eUndefined;
	}

	auto VKGPUContext::findDepthFormat() const -> vk::Format
	{
		return m_depthFormat;
	}

	auto VKGPUContext::hasStencilComponent(const vk::Format p_format) const -> bool
	{
		return p_format == vk::Format::eD32SfloatS8Uint || p_format == vk::Format::eD24UnormS8Uint;
	}

	auto VKGPUContext::isDepthFormat(const vk::Format p_format) const -> bool
	{
		return p_format == vk::Format::eD16Unorm || p_format == vk::Format::eD16UnormS8Uint || p_format == vk::Format::eD24UnormS8Uint || p_format ==
			   vk::Format::eD32Sfloat || p_format == vk::Format::eD32SfloatS8Uint;
	}

	auto VKGPUContext::getMaxUsableSampleCount() const -> vk::SampleCountFlagBits
	{
		return m_maxUsableSampleCount;
	}
}
