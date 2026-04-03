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

	vk::raii::Queue &VKGPUContext::getTransferQueue()
	{
		return m_transferQueue;
	}

	vk::raii::Queue &VKGPUContext::getComputeQueue()
	{
		return m_computeQueue;
	}

	const VKGPUContext::QueueFamilyIndices &VKGPUContext::getQueueFamilyIndices() const
	{
		return m_queueFamilyIndices;
	}

	vk::raii::SurfaceKHR &VKGPUContext::getSurface()
	{
		return m_surface;
	}

	vk::raii::CommandPool &VKGPUContext::getGraphicsCommandPool()
	{
		return m_graphicsCommandPool;
	}

	vk::raii::CommandPool &VKGPUContext::getTransferCommandPool()
	{
		return m_transferCommandPool;
	}

	vk::raii::CommandPool &VKGPUContext::getComputeCommandPool()
	{
		return m_computeCommandPool;
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

		#ifndef NDEBUG

		LOG_INFO("Available instance extensions:");
		for (auto &prop: extension_props)
			LOG_INFO("\t{}", prop.extensionName.data());
		LOG_INFO("");

		#endif

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

		#ifndef NDEBUG

		auto props = m_currentPhysicalDevice.getProperties();
		LOG_INFO("Using physical device: {} | Device ID: {}\n", props.deviceName.data(), props.deviceID);

		LOG_INFO("Available device extensions:");
		auto extension_props = m_currentPhysicalDevice.enumerateDeviceExtensionProperties();
		for (auto ext: extension_props)
			LOG_INFO("\t{}", ext.extensionName.data());
		LOG_INFO("");

		#endif
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
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> feature_chain{{}, {}, {}, {}};
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore                   = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering                    = true;
		feature_chain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};

		// Create the graphics and present queue
		// The graphics queue should be the same as the present one
		auto &graphics_queue_create_info            = queue_create_infos.emplace_back();
		graphics_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.graphics;
		graphics_queue_create_info.queueCount       = 1;
		constexpr float32 queue_priority            = 1.0f;
		graphics_queue_create_info.pQueuePriorities = &queue_priority;

		// Create the transfer queue
		auto &transfer_queue_create_info            = queue_create_infos.emplace_back();
		transfer_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.transfer;
		transfer_queue_create_info.queueCount       = 1;
		transfer_queue_create_info.pQueuePriorities = &queue_priority;

		if (m_queueFamilyIndices.graphics != m_queueFamilyIndices.compute)
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

		#ifndef NDEBUG
		LOG_TRACE("Graphics queue family index {}", m_queueFamilyIndices.graphics);
		LOG_TRACE("Transfer queue family index {}", m_queueFamilyIndices.transfer);
		LOG_TRACE("Compute queue family index {}", m_queueFamilyIndices.compute);
		#endif
	}

	void VKGPUContext::_createCommandPools()
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

	bool VKGPUContext::_isDeviceSuitable(const vk::raii::PhysicalDevice &p_physical_device) const
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
		std::vector required_device_extensions{vk::KHRSwapchainExtensionName, vk::KHRDynamicRenderingExtensionName, vk::KHRTimelineSemaphoreExtensionName};

		// Checks if all the required extensions are present in the available_device_extensions vector.
		auto available_device_extensions             = p_physical_device.enumerateDeviceExtensionProperties();
		bool supports_all_required_device_extensions = std::ranges::all_of(required_device_extensions, [available_device_extensions](const auto &required_ext)
		{
			return std::ranges::any_of(available_device_extensions, [&required_ext](const auto &available_ext)
			{
				return std::strcmp(available_ext.extensionName, required_ext) == 0;
			});
		});

		auto features = p_physical_device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

		bool supports_required_features = features.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore && features.get<vk::PhysicalDeviceVulkan13Features>().
										  dynamicRendering && features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

		return vulkan_1_3_support && supports_graphics && supports_compute && supports_all_required_device_extensions && supports_required_features;
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

	uint32 VKGPUContext::findMemoryType(uint32 p_type_filter, vk::MemoryPropertyFlags p_properties) const
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

	void VKGPUContext::createBuffer(vk::DeviceSize    p_size, vk::BufferUsageFlags          p_usage_flags, vk::MemoryPropertyFlags p_memory_properties,
									vk::raii::Buffer &p_out_buffer, vk::raii::DeviceMemory &p_out_memory) const
	{
		vk::BufferCreateInfo buffer_create_info{};
		buffer_create_info.size        = p_size;
		buffer_create_info.usage       = p_usage_flags;
		buffer_create_info.sharingMode = vk::SharingMode::eConcurrent;

		p_out_buffer = {m_device, buffer_create_info};

		vk::MemoryRequirements memory_requirements = p_out_buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, p_memory_properties);
		memory_allocate_info.allocationSize  = memory_requirements.size;

		p_out_memory = {m_device, memory_allocate_info};

		p_out_buffer.bindMemory(p_out_memory, 0u);
	}

	void VKGPUContext::copyBuffer(vk::raii::Buffer &p_src_buffer, vk::raii::Buffer &p_dst_buffer, vk::DeviceSize p_size) const
	{
		vk::BufferCopy buffer_copy{};
		buffer_copy.size      = static_cast<uint32>(p_size);
		buffer_copy.srcOffset = 0;
		buffer_copy.dstOffset = 0;

		vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.commandPool        = m_transferCommandPool;
		command_buffer_allocate_info.level              = vk::CommandBufferLevel::ePrimary;

		vk::raii::CommandBuffer command_buffer{std::move(m_device.allocateCommandBuffers(command_buffer_allocate_info).front())};

		vk::FenceCreateInfo fence_create_info{};
		vk::raii::Fence     wait_fence{m_device, fence_create_info};

		vk::CommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		command_buffer.begin(command_buffer_begin_info);
		command_buffer.copyBuffer(p_src_buffer, p_dst_buffer, buffer_copy);
		command_buffer.end();

		vk::SubmitInfo submit_info{};
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers    = &*command_buffer;
		m_transferQueue.submit(submit_info, *wait_fence);

		vk::Result res = m_device.waitForFences(*wait_fence, true, UINT64_MAX);
		if (res != vk::Result::eSuccess)
			TST_ASSERT_MSG(false, "Failed to wait for Fence");
	}
}
