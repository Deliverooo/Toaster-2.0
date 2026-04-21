#include <QVulkanWindow>

#include "client_application.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

auto cstringArrayToVector(toaster::CString *p_arr, uint32 p_size) -> std::vector<toaster::CString>
{
	std::vector<toaster::CString> vec{p_size};
	for (uint32 i{0u}; i < p_size; ++i)
		vec.emplace_back(p_arr[i]);
	return vec;
}

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
int32 main(int32 p_argc, char **p_argv) // Maybe_todo, Forward these parameters to the application for it to handle
{
	#endif

	QGuiApplication app{p_argc, p_argv};

	uint32                                         extension_count{0u};
	auto                                           required_extensions{cstringArrayToVector(glfwGetRequiredInstanceExtensions(&extension_count), extension_count)};
	toaster::gpu::VKInstanceSpecInfo::ExtensionSet instance_extensions{required_extensions.begin(), required_extensions.end()};

	instance_extensions.insert(vk::KHRSurfaceExtensionName);
	toaster::gpu::VKInstanceSpecInfo vk_instance_spec_info{};
	vk_instance_spec_info.appName            = "Toaster-2.0 -> Vulkan QT";
	vk_instance_spec_info.requiredExtensions = instance_extensions;
	toaster::gpu::VKInstance vk_instance{vk_instance_spec_info};

	toaster::gpu::VKPhysicalDeviceSpecInfo vk_physical_device_spec_info{};
	vk_physical_device_spec_info.requiredExtensions = {
		vk::KHRSwapchainExtensionName,
		vk::KHRDynamicRenderingExtensionName,
		vk::KHRTimelineSemaphoreExtensionName,
		vk::EXTCustomBorderColorExtensionName,
		vk::KHRMaintenance6ExtensionName,
		vk::KHRLoadStoreOpNoneExtensionName
	};
	toaster::gpu::VKPhysicalDevice vk_physical_device{&vk_instance, vk_physical_device_spec_info};

	#if 0
	VkSurfaceKHR surface; if (glfwCreateWindowSurface(*vk_instance.getVulkanInstance(), m_window, nullptr, &surface) != VK_SUCCESS)
	{
		LOG_ERROR("Failed to create window surface");
		system("pause");
		TST_ASSERT(false);
	}
	#endif

	toaster::gpu::VKLogicalDeviceSpecInfo vk_logical_device_spec_info{};
	vk_logical_device_spec_info.surface            = nullptr;
	vk_logical_device_spec_info.requiredExtensions = {
		vk::KHRSwapchainExtensionName,
		vk::KHRDynamicRenderingExtensionName,
		vk::KHRTimelineSemaphoreExtensionName,
		vk::EXTCustomBorderColorExtensionName,
		vk::KHRMaintenance6ExtensionName,
		vk::KHRLoadStoreOpNoneExtensionName
	};

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> feature_chain{{}, {}, {}, {}};
	feature_chain.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy                 = true;
	feature_chain.get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading                 = true;
	feature_chain.get<vk::PhysicalDeviceFeatures2>().features.fillModeNonSolid                  = true;
	feature_chain.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore                   = true;
	feature_chain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering                    = true;
	feature_chain.get<vk::PhysicalDeviceVulkan13Features>().synchronization2                    = true;
	feature_chain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;
	vk_logical_device_spec_info.pNext                                                           = feature_chain.get<vk::PhysicalDeviceFeatures2>();
	toaster::gpu::VKLogicalDevice vk_logical_device{&vk_physical_device, vk_logical_device_spec_info};

	QVulkanInstance qvk_instance{};
	qvk_instance.setVkInstance(*vk_instance.getVulkanInstance());

	QVulkanWindow vk_window{};

	vk_window.setTitle("Toaster - QT test :)");
	vk_window.show();

	return app.exec();
}
