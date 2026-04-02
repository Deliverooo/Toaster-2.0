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
			// _createSwapchain();
			// _createImageViews();
			// _createSyncObjects();
			_createGraphicsPipeline();
			_createCommandPool();
			_createCommandBuffer();
		}
		catch (const vk::SystemError &err)
		{
			LOG_ERROR("Vulkan error: {}", err.what());
		}
	}

	VKGPUContext::~VKGPUContext() noexcept
	{
	}

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

	vk::raii::CommandBuffer &VKGPUContext::getCommandBuffer(uint32 p_index)
	{
		return m_commandBuffers[p_index];
	}

	vk::raii::Pipeline &VKGPUContext::getGraphicsPipeline()
	{
		return m_graphicsPipeline;
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

	#if 0

	void VKGPUContext::_createSwapchain()
	{
		vk::SurfaceCapabilitiesKHR surface_caps = m_currentPhysicalDevice.getSurfaceCapabilitiesKHR(m_surface);

		auto available_surface_formats = m_currentPhysicalDevice.getSurfaceFormatsKHR(m_surface);
		auto available_present_modes   = m_currentPhysicalDevice.getSurfacePresentModesKHR(m_surface);

		m_swapchainSurfaceFormat = _chooseSwapchainSurfaceFormat(available_surface_formats);
		m_swapchainExtent        = _chooseSwapchainExtent(surface_caps);

		uint32 min_image_count = _chooseSwapchainMinImageCount(surface_caps);

		vk::SwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.surface          = m_surface;
		swapchain_create_info.minImageCount    = min_image_count;
		swapchain_create_info.imageFormat      = m_swapchainSurfaceFormat.format;
		swapchain_create_info.imageColorSpace  = m_swapchainSurfaceFormat.colorSpace;
		swapchain_create_info.imageExtent      = m_swapchainExtent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
		swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
		swapchain_create_info.preTransform     = surface_caps.currentTransform;
		swapchain_create_info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchain_create_info.presentMode      = _chooseSwapchainPresentMode(available_present_modes);
		swapchain_create_info.clipped          = true;

		m_swapchain       = {m_device, swapchain_create_info};
		m_swapchainImages = m_swapchain.getImages();
	} void VKGPUContext::_createImageViews()
	{
		vk::ImageViewCreateInfo image_view_create_info{};
		image_view_create_info.viewType         = vk::ImageViewType::e2D;
		image_view_create_info.format           = m_swapchainSurfaceFormat.format;
		image_view_create_info.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
		image_view_create_info.components       = {
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		};
		for (auto &image: m_swapchainImages)
		{
			image_view_create_info.image = image;
			m_swapchainImageViews.emplace_back(m_device, image_view_create_info);
		}
	} void VKGPUContext::_createSyncObjects()
	{
		for (uint32 i{0u}; i < m_swapchainImageViews.size(); ++i)
			m_renderFinishedSemaphores.emplace_back(m_device, vk::SemaphoreCreateInfo{});

		for (uint32 i{0u}; i < c_maxFramesInFlight; ++i)
		{
			m_imageAvailableSemaphores.emplace_back(m_device, vk::SemaphoreCreateInfo{});

			vk::FenceCreateInfo fence_create_info{};
			fence_create_info.flags = vk::FenceCreateFlagBits::eSignaled;
			m_inFlightFences.emplace_back(m_device, fence_create_info);
		}
	}

	#endif
	void VKGPUContext::_createGraphicsPipeline()
	{
		vk::raii::ShaderModule vertex_shader_module = _createShaderModule(io::filesystem::readBinary("shaders/test.vert.glsl.spv"));
		vk::raii::ShaderModule pixel_shader_module  = _createShaderModule(io::filesystem::readBinary("shaders/test.pixel.glsl.spv"));

		vk::PipelineShaderStageCreateInfo vertex_shader_stage_create_info{};
		vertex_shader_stage_create_info.stage  = vk::ShaderStageFlagBits::eVertex;
		vertex_shader_stage_create_info.module = vertex_shader_module;
		vertex_shader_stage_create_info.pName  = "main";

		vk::PipelineShaderStageCreateInfo pixel_shader_stage_create_info{};
		pixel_shader_stage_create_info.stage  = vk::ShaderStageFlagBits::eFragment;
		pixel_shader_stage_create_info.module = pixel_shader_module;
		pixel_shader_stage_create_info.pName  = "main";

		vk::PipelineShaderStageCreateInfo shader_stage_create_infos[] = {vertex_shader_stage_create_info, pixel_shader_stage_create_info};

		vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info{};

		vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
		input_assembly_state_create_info.topology = vk::PrimitiveTopology::eTriangleList;

		vk::PipelineViewportStateCreateInfo viewport_state_create_info{};
		viewport_state_create_info.viewportCount = 1;
		viewport_state_create_info.scissorCount  = 1;

		vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info{};
		rasterization_state_create_info.depthClampEnable        = false;
		rasterization_state_create_info.rasterizerDiscardEnable = false;
		rasterization_state_create_info.polygonMode             = vk::PolygonMode::eFill;
		rasterization_state_create_info.cullMode                = vk::CullModeFlagBits::eBack;
		rasterization_state_create_info.frontFace               = vk::FrontFace::eClockwise;
		rasterization_state_create_info.depthBiasEnable         = false;
		rasterization_state_create_info.lineWidth               = 1.0f;

		vk::PipelineColorBlendAttachmentState colour_blend_attachment_state{};
		colour_blend_attachment_state.blendEnable    = false;
		colour_blend_attachment_state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
													   vk::ColorComponentFlagBits::eA;

		vk::PipelineColorBlendStateCreateInfo colour_blend_state_create_info{};
		colour_blend_state_create_info.logicOpEnable   = false;
		colour_blend_state_create_info.logicOp         = vk::LogicOp::eCopy;
		colour_blend_state_create_info.attachmentCount = 1;
		colour_blend_state_create_info.pAttachments    = &colour_blend_attachment_state;

		std::array                         dynamic_states{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{};
		dynamic_state_create_info.pDynamicStates    = dynamic_states.data();
		dynamic_state_create_info.dynamicStateCount = dynamic_states.size();

		vk::PipelineMultisampleStateCreateInfo multisample_state_create_info{};
		multisample_state_create_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisample_state_create_info.sampleShadingEnable  = false;

		vk::Format                      attachment_format = vk::Format::eR8G8B8A8Srgb;
		vk::PipelineRenderingCreateInfo rendering_create_info{};
		rendering_create_info.colorAttachmentCount = 1;
		// rendering_create_info.pColorAttachmentFormats = &m_swapchainSurfaceFormat.format;
		rendering_create_info.pColorAttachmentFormats = &attachment_format;

		vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.setLayoutCount         = 0;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		m_pipelineLayout                                   = {m_device, pipeline_layout_create_info};

		vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info{};
		graphics_pipeline_create_info.stageCount          = 2;
		graphics_pipeline_create_info.pStages             = shader_stage_create_infos;
		graphics_pipeline_create_info.pVertexInputState   = &vertex_input_state_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
		graphics_pipeline_create_info.pViewportState      = &viewport_state_create_info;
		graphics_pipeline_create_info.pMultisampleState   = &multisample_state_create_info;
		graphics_pipeline_create_info.pColorBlendState    = &colour_blend_state_create_info;
		graphics_pipeline_create_info.pDynamicState       = &dynamic_state_create_info;
		graphics_pipeline_create_info.layout              = m_pipelineLayout;
		graphics_pipeline_create_info.renderPass          = nullptr;
		graphics_pipeline_create_info.pNext               = &rendering_create_info;

		m_graphicsPipeline = {m_device, nullptr, graphics_pipeline_create_info};
	}

	void VKGPUContext::_createCommandPool()
	{
		vk::CommandPoolCreateInfo command_pool_create_info{};
		command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.graphics;
		command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_commandPool = {m_device, command_pool_create_info};
	}

	void VKGPUContext::_createCommandBuffer()
	{
		vk::CommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.commandPool        = m_commandPool;
		command_buffer_allocate_info.commandBufferCount = c_maxFramesInFlight;
		command_buffer_allocate_info.level              = vk::CommandBufferLevel::ePrimary;

		m_commandBuffers = vk::raii::CommandBuffers{m_device, command_buffer_allocate_info};
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
			return static_cast<bool>(queue_family.queueFlags & vk::QueueFlagBits::eGraphics);
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

	vk::raii::ShaderModule VKGPUContext::_createShaderModule(const std::vector<uint8> &p_code)
	{
		vk::ShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.codeSize = p_code.size();
		shader_module_create_info.pCode    = reinterpret_cast<const uint32 *>(p_code.data());

		return {m_device, shader_module_create_info};
	}

	void VKGPUContext::transitionImageLayout(vk::Image &             p_image, uint32 p_frame_index, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
											 vk::AccessFlags2        p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
											 vk::PipelineStageFlags2 p_dst_stage_mask)
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

		m_commandBuffers[p_frame_index].pipelineBarrier2(dependency_info);
	}
}
