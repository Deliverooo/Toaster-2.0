#include "vk_gpu_context.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

namespace toaster::gpu
{
	VKGPUContext::VKGPUContext(GLFWwindow *p_window)
	{
		vk::ApplicationInfo appInfo{};
		appInfo.pApplicationName = "Toaster";
		appInfo.pEngineName      = "Toaster";
		appInfo.apiVersion       = vk::ApiVersion14;

		#ifdef WIN32
		#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
		#endif
		std::vector<const char *> instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
		instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Very little performance hit, can be used in Release.

		#if ENABLE_VALIDATION_LAYERS
		instanceExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		instanceExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		#endif

		vk::ValidationFeatureEnableEXT enables[] = {vk::ValidationFeatureEnableEXT::eBestPractices};
		vk::ValidationFeaturesEXT      features{};
		features.enabledValidationFeatureCount = 1;
		features.pEnabledValidationFeatures    = enables;

		vk::InstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.pApplicationInfo        = &appInfo;
		instanceCreateInfo.enabledExtensionCount   = static_cast<uint32>(instanceExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

		#if ENABLE_VALIDATION_LAYERS
		auto validationLayerName = "VK_LAYER_KHRONOS_validation";

		auto instanceLayerProperties = vk::enumerateInstanceLayerProperties();
		bool validationLayerPresent = false;

		LOG_INFO("Vulkan Instance Layers:");
		for (const auto& layer : instanceLayerProperties)
		{
			LOG_INFO("{0}", layer.layerName.data());
			if (std::strcmp(layer.layerName, validationLayerName) == 0)
			{
				validationLayerPresent = true;
				break;
			}
		}
		if (validationLayerPresent)
		{
			instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
			instanceCreateInfo.enabledLayerCount   = 1;
		}
		else
		{
			LOG_ERROR("Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled");
		}
		#endif

		TST_VK_CHECK_RESULT(vk::createInstance(&instanceCreateInfo, nullptr, &m_vulkanInstance));
	}

	VKGPUContext::~VKGPUContext()
	{
	}
}
